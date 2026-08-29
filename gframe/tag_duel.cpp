#include <array>
#include <utility>
#include "config.h"
#include "tag_duel.h"
#include "netserver.h"
#include "game.h"
#include "data_manager.h"

namespace ygo
{

#ifdef YGOPRO_SERVER_MODE
    extern unsigned short replay_mode;
#endif
    TagDuel::TagDuel()
    {
        for (int i = 0; i < 4; ++i)
        {
            players[i] = 0;
            ready[i] = false;
            surrender[i] = false;
#ifdef YGOPRO_SERVER_MODE
            previous_opponent[i] = 0xff;
#endif
        }
    }
    TagDuel::~TagDuel()
    {
    }
    void TagDuel::AbortDuelWithChat(const char16_t *message)
    {
        unsigned char scc[SIZE_STOC_CHAT];
        const auto message_size = static_cast<int>((std::char_traits<char16_t>::length(message) + 1) * sizeof(char16_t));
        const auto scc_size = NetServer::CreateSystemChatPacket(reinterpret_cast<const unsigned char *>(message), message_size, scc);
        if (scc_size)
        {
            NetServer::SendBufferToPlayer(players[0], STOC_CHAT, scc, scc_size);
            NetServer::ReSendToPlayer(players[1]);
            NetServer::ReSendToPlayer(players[2]);
            NetServer::ReSendToPlayer(players[3]);
        }
        EndDuel();
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::send_system_chat(DuelPlayer *recipient, const wchar_t *message)
    {
        uint16_t message_utf16[LEN_CHAT_MSG]{};
        size_t utf16_length = 0;
        for (; *message; ++message)
        {
            const uint32_t codepoint = static_cast<uint32_t>(*message);
            const size_t code_unit_count = codepoint <= 0xffff ? 1 : codepoint <= 0x10ffff ? 2
                                                                                           : 0;
            if (!code_unit_count || utf16_length + code_unit_count >= LEN_CHAT_MSG)
                return;
            if (code_unit_count == 1)
            {
                message_utf16[utf16_length++] = static_cast<uint16_t>(codepoint);
            }
            else
            {
                const uint32_t surrogate = codepoint - 0x10000;
                message_utf16[utf16_length++] = static_cast<uint16_t>(0xd800 + (surrogate >> 10));
                message_utf16[utf16_length++] = static_cast<uint16_t>(0xdc00 + (surrogate & 0x3ff));
            }
        }
        unsigned char scc[SIZE_STOC_CHAT];
        const auto message_size = static_cast<int>((utf16_length + 1) * sizeof(uint16_t));
        const auto scc_size = NetServer::CreateSystemChatPacket(reinterpret_cast<const unsigned char *>(message_utf16), message_size, scc);
        if (scc_size)
            NetServer::SendBufferToPlayer(recipient, STOC_CHAT, scc, scc_size);
    }
    void TagDuel::send_battle_system_chat(unsigned char battle_id, uint32_t string_id)
    {
        if (battle_id >= active_battle_count)
            return;
        const wchar_t *message = dataManager.GetSysString(string_id);
        send_system_chat(players[battle_pairs[battle_id][0]], message);
        send_system_chat(players[battle_pairs[battle_id][1]], message);
    }
    int TagDuel::remap_field_player(int player) const
    {
        if (active_field < HOME_FIELD_COUNT)
        {
            if (player == 1)
            {
                return -1;
            }
            return active_field;
        }
        if (active_field >= FIELD_COUNT || (player != 0 && player != 1))
            return -1;
        const int battle_id = active_field - HOME_FIELD_COUNT;
        return battle_pairs[battle_id][player];
    }
    bool TagDuel::IsBattleField() const
    {
        return active_field >= HOME_FIELD_COUNT && active_field < HOME_FIELD_COUNT + BATTLE_FIELD_COUNT;
    }
    void TagDuel::BcastObs(unsigned char *buf, unsigned int size, bool send_to_recorder, int except_player)
    {
        if (IsBattleField())
        {
            const unsigned char battle_id = active_field - HOME_FIELD_COUNT;
            if (except_player != battle_pairs[battle_id][0])
                NetServer::SendBufferToPlayer(players[battle_pairs[battle_id][0]], STOC_GAME_MSG, buf, size);
            if (except_player != battle_pairs[battle_id][1])
                NetServer::SendBufferToPlayer(players[battle_pairs[battle_id][1]], STOC_GAME_MSG, buf, size);
        }
        else
        {
            if (except_player != active_field)
                NetServer::SendBufferToPlayer(players[active_field], STOC_GAME_MSG, buf, size);
        }
        for (auto oit = observers.begin(); oit != observers.end(); ++oit)
            NetServer::ReSendToPlayer(*oit);
        if (send_to_recorder)
        {
            NetServer::ReSendToPlayers(cache_recorder, replay_recorder);
        }
    }
#endif
    void TagDuel::Chat(DuelPlayer *dp, unsigned char *pdata, int len)
    {
        unsigned char scc[SIZE_STOC_CHAT];
        const auto scc_size = NetServer::CreateChatPacket(pdata, len, scc, dp->type);
        if (!scc_size)
            return;
        for (int i = 0; i < 4; ++i)
            NetServer::SendBufferToPlayer(players[i], STOC_CHAT, scc, scc_size);
        for (auto pit = observers.begin(); pit != observers.end(); ++pit)
            NetServer::ReSendToPlayer(*pit);
#ifdef YGOPRO_SERVER_MODE
        if (cache_recorder)
            NetServer::ReSendToPlayer(cache_recorder);
        if (replay_recorder && replay_mode & REPLAY_MODE_INCLUDE_CHAT)
            NetServer::ReSendToPlayer(replay_recorder);
#endif
    }
    void TagDuel::JoinGame(DuelPlayer *dp, unsigned char *pdata, bool is_creater)
    {
#ifdef YGOPRO_SERVER_MODE
        bool is_recorder = false;
#endif
        if (!is_creater)
        {
            if (dp->game && dp->type != 0xff)
            {
                STOC_ErrorMsg scem;
                scem.msg = ERRMSG_JOINERROR;
                scem.code = 0;
                NetServer::SendPacketToPlayer(dp, STOC_ERROR_MSG, scem);
                NetServer::DisconnectPlayer(dp);
                return;
            }
            CTOS_JoinGame packet;
            std::memcpy(&packet, pdata, sizeof packet);
            auto pkt = &packet;
            if (pkt->version != PRO_VERSION)
            {
                STOC_ErrorMsg scem;
                scem.msg = ERRMSG_VERERROR;
                scem.code = PRO_VERSION;
                NetServer::SendPacketToPlayer(dp, STOC_ERROR_MSG, scem);
                NetServer::DisconnectPlayer(dp);
                return;
            }
            wchar_t jpass[20];
            BufferIO::NullTerminate(pkt->pass);
            BufferIO::CopyCharArray(pkt->pass, jpass);
#ifdef YGOPRO_SERVER_MODE
            if (!std::wcscmp(jpass, L"the Big Brother") && !cache_recorder)
            {
                is_recorder = true;
                cache_recorder = dp;
            }
#ifndef YGOPRO_SERVER_MODE_DISABLE_CLOUD_REPLAY
            if (!std::wcscmp(jpass, L"Marshtomp") && !replay_recorder)
            {
                is_recorder = true;
                replay_recorder = dp;
            }
#endif // YGOPRO_SERVER_MODE_DISABLE_CLOUD_REPLAY
#else
            if (std::wcscmp(jpass, pass))
            {
                STOC_ErrorMsg scem;
                scem.msg = ERRMSG_JOINERROR;
                scem.code = 1;
                NetServer::SendPacketToPlayer(dp, STOC_ERROR_MSG, scem);
                return;
            }
#endif // YGOPRO_SERVER_MODE
        }
        dp->game = this;
        if (!players[0] && !players[1] && !players[2] && !players[3] && observers.size() == 0)
            host_player = dp;
        STOC_JoinGame scjg;
        scjg.info = host_info;
        STOC_TypeChange sctc;
        sctc.type = (host_player == dp) ? 0x10 : 0;
#ifdef YGOPRO_SERVER_MODE
        if (is_recorder)
        {
            dp->type = 9;
            sctc.type = NETPLAYER_TYPE_OBSERVER;
        }
        else
#endif
            if (!players[0] || !players[1] || !players[2] || !players[3])
        {
            STOC_HS_PlayerEnter scpe;
            BufferIO::CopyCharArray(dp->name, scpe.name);
            if (!players[0])
                scpe.pos = 0;
            else if (!players[1])
                scpe.pos = 1;
            else if (!players[2])
                scpe.pos = 2;
            else
                scpe.pos = 3;
            for (int i = 0; i < 4; ++i)
                if (players[i])
                    NetServer::SendPacketToPlayer(players[i], STOC_HS_PLAYER_ENTER, scpe);
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::SendPacketToPlayer(*pit, STOC_HS_PLAYER_ENTER, scpe);
#ifdef YGOPRO_SERVER_MODE
            if (cache_recorder)
                NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_PLAYER_ENTER, scpe);
            if (replay_recorder)
                NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_PLAYER_ENTER, scpe);
#endif
            players[scpe.pos] = dp;
            dp->type = scpe.pos;
            sctc.type |= scpe.pos;
        }
        else
        {
            observers.insert(dp);
            dp->type = NETPLAYER_TYPE_OBSERVER;
            sctc.type |= NETPLAYER_TYPE_OBSERVER;
            STOC_HS_WatchChange scwc;
            scwc.watch_count = (unsigned short)observers.size();
            for (int i = 0; i < 4; ++i)
                if (players[i])
                    NetServer::SendPacketToPlayer(players[i], STOC_HS_WATCH_CHANGE, scwc);
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::SendPacketToPlayer(*pit, STOC_HS_WATCH_CHANGE, scwc);
#ifdef YGOPRO_SERVER_MODE
            if (cache_recorder)
                NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_WATCH_CHANGE, scwc);
            if (replay_recorder)
                NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_WATCH_CHANGE, scwc);
#endif
        }
        NetServer::SendPacketToPlayer(dp, STOC_JOIN_GAME, scjg);
        NetServer::SendPacketToPlayer(dp, STOC_TYPE_CHANGE, sctc);
        for (int i = 0; i < 4; ++i)
            if (players[i])
            {
                STOC_HS_PlayerEnter scpe;
                BufferIO::CopyCharArray(players[i]->name, scpe.name);
                scpe.pos = i;
                NetServer::SendPacketToPlayer(dp, STOC_HS_PLAYER_ENTER, scpe);
                if (ready[i])
                {
                    STOC_HS_PlayerChange scpc;
                    scpc.status = (i << 4) | PLAYERCHANGE_READY;
                    NetServer::SendPacketToPlayer(dp, STOC_HS_PLAYER_CHANGE, scpc);
                }
            }
        if (observers.size())
        {
            STOC_HS_WatchChange scwc;
            scwc.watch_count = (unsigned short)observers.size();
            NetServer::SendPacketToPlayer(dp, STOC_HS_WATCH_CHANGE, scwc);
        }
    }
    void TagDuel::LeaveGame(DuelPlayer *dp)
    {
        if (dp == host_player)
        {
#ifdef YGOPRO_SERVER_MODE
            int host_pos;
            if (players[0] && dp->type != 0)
            {
                host_pos = 0;
                host_player = players[0];
            }
            else if (players[1] && dp->type != 1)
            {
                host_pos = 1;
                host_player = players[1];
            }
            else if (players[2] && dp->type != 2)
            {
                host_pos = 2;
                host_player = players[2];
            }
            else if (players[3] && dp->type != 3)
            {
                host_pos = 3;
                host_player = players[3];
            }
            else
            {
                EndDuel();
                NetServer::StopServer();
                return;
            }
            if (duel_stage == DUEL_STAGE_BEGIN)
            {
                ready[host_pos] = false;
                STOC_TypeChange sctc;
                sctc.type = 0x10 | host_pos;
                NetServer::SendPacketToPlayer(players[host_pos], STOC_TYPE_CHANGE, sctc);
            }
        }
        if (dp->type == NETPLAYER_TYPE_OBSERVER)
        {
#else
            EndDuel();
            NetServer::StopServer();
        }
        else if (dp->type == NETPLAYER_TYPE_OBSERVER)
        {
#endif // YGOPRO_SERVER_MODE
            observers.erase(dp);
            if (duel_stage == DUEL_STAGE_BEGIN)
            {
                STOC_HS_WatchChange scwc;
                scwc.watch_count = (unsigned short)observers.size();
                for (int i = 0; i < 4; ++i)
                    if (players[i])
                        NetServer::SendPacketToPlayer(players[i], STOC_HS_WATCH_CHANGE, scwc);
                for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                    NetServer::SendPacketToPlayer(*pit, STOC_HS_WATCH_CHANGE, scwc);
#ifdef YGOPRO_SERVER_MODE
                if (cache_recorder)
                    NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_WATCH_CHANGE, scwc);
                if (replay_recorder)
                    NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_WATCH_CHANGE, scwc);
#endif
            }
            NetServer::DisconnectPlayer(dp);
        }
        else
        {
            if (duel_stage == DUEL_STAGE_BEGIN)
            {
                STOC_HS_PlayerChange scpc;
                players[dp->type] = 0;
                ready[dp->type] = false;
                scpc.status = (dp->type << 4) | PLAYERCHANGE_LEAVE;
                for (int i = 0; i < 4; ++i)
                    if (players[i])
                        NetServer::SendPacketToPlayer(players[i], STOC_HS_PLAYER_CHANGE, scpc);
                for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                    NetServer::SendPacketToPlayer(*pit, STOC_HS_PLAYER_CHANGE, scpc);
#ifdef YGOPRO_SERVER_MODE
                if (cache_recorder)
                    NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_PLAYER_CHANGE, scpc);
                if (replay_recorder)
                    NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_PLAYER_CHANGE, scpc);
#endif
            }
            else if (duel_stage != DUEL_STAGE_END)
            {
                EndDuel();
                DuelEndProc();
            }
            NetServer::DisconnectPlayer(dp);
        }
    }
    void TagDuel::ToDuelist(DuelPlayer *dp)
    {
        if (players[0] && players[1] && players[2] && players[3])
            return;
        if (dp->type == NETPLAYER_TYPE_OBSERVER)
        {
            observers.erase(dp);
            STOC_HS_PlayerEnter scpe;
            BufferIO::CopyCharArray(dp->name, scpe.name);
            if (!players[0])
                dp->type = 0;
            else if (!players[1])
                dp->type = 1;
            else if (!players[2])
                dp->type = 2;
            else
                dp->type = 3;
            players[dp->type] = dp;
            scpe.pos = dp->type;
            STOC_HS_WatchChange scwc;
            scwc.watch_count = (unsigned short)observers.size();
            for (int i = 0; i < 4; ++i)
                if (players[i])
                {
                    NetServer::SendPacketToPlayer(players[i], STOC_HS_PLAYER_ENTER, scpe);
                    NetServer::SendPacketToPlayer(players[i], STOC_HS_WATCH_CHANGE, scwc);
                }
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
            {
                NetServer::SendPacketToPlayer(*pit, STOC_HS_PLAYER_ENTER, scpe);
                NetServer::SendPacketToPlayer(*pit, STOC_HS_WATCH_CHANGE, scwc);
            }
#ifdef YGOPRO_SERVER_MODE
            if (cache_recorder)
            {
                NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_PLAYER_ENTER, scpe);
                NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_WATCH_CHANGE, scwc);
            }
            if (replay_recorder)
            {
                NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_PLAYER_ENTER, scpe);
                NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_WATCH_CHANGE, scwc);
            }
#endif
            STOC_TypeChange sctc;
            sctc.type = (dp == host_player ? 0x10 : 0) | dp->type;
            NetServer::SendPacketToPlayer(dp, STOC_TYPE_CHANGE, sctc);
        }
        else
        {
            if (ready[dp->type])
                return;
            unsigned char dptype = (dp->type + 1) % 4;
            while (players[dptype])
                dptype = (dptype + 1) % 4;
            STOC_HS_PlayerChange scpc;
            scpc.status = (dp->type << 4) | dptype;
            for (int i = 0; i < 4; ++i)
                if (players[i])
                    NetServer::SendPacketToPlayer(players[i], STOC_HS_PLAYER_CHANGE, scpc);
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::SendPacketToPlayer(*pit, STOC_HS_PLAYER_CHANGE, scpc);
#ifdef YGOPRO_SERVER_MODE
            if (cache_recorder)
                NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_PLAYER_CHANGE, scpc);
            if (replay_recorder)
                NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_PLAYER_CHANGE, scpc);
#endif
            STOC_TypeChange sctc;
            sctc.type = (dp == host_player ? 0x10 : 0) | dptype;
            NetServer::SendPacketToPlayer(dp, STOC_TYPE_CHANGE, sctc);
            players[dptype] = dp;
            players[dp->type] = 0;
            dp->type = dptype;
        }
    }
    void TagDuel::ToObserver(DuelPlayer *dp)
    {
        if (dp->type > 3)
            return;
        STOC_HS_PlayerChange scpc;
        scpc.status = (dp->type << 4) | PLAYERCHANGE_OBSERVE;
        for (int i = 0; i < 4; ++i)
            if (players[i])
                NetServer::SendPacketToPlayer(players[i], STOC_HS_PLAYER_CHANGE, scpc);
        for (auto pit = observers.begin(); pit != observers.end(); ++pit)
            NetServer::SendPacketToPlayer(*pit, STOC_HS_PLAYER_CHANGE, scpc);
#ifdef YGOPRO_SERVER_MODE
        if (cache_recorder)
            NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_PLAYER_CHANGE, scpc);
        if (replay_recorder)
            NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_PLAYER_CHANGE, scpc);
#endif
        players[dp->type] = 0;
        ready[dp->type] = false;
        dp->type = NETPLAYER_TYPE_OBSERVER;
        observers.insert(dp);
        STOC_TypeChange sctc;
        sctc.type = (dp == host_player ? 0x10 : 0) | dp->type;
        NetServer::SendPacketToPlayer(dp, STOC_TYPE_CHANGE, sctc);
    }
    void TagDuel::PlayerReady(DuelPlayer *dp, bool is_ready)
    {
        if (dp->type > 3 || ready[dp->type] == is_ready)
            return;
        if (is_ready)
        {
            uint32_t deckerror = 0;
            if (!host_info.no_check_deck)
            {
                if (deck_error[dp->type])
                {
                    deckerror = (DECKERROR_UNKNOWNCARD << 28) | deck_error[dp->type];
                }
                else
                {
                    deckerror = deckManager.CheckDeck(pdeck[dp->type], host_info.lflist, host_info.rule);
                }
            }
            if (deckerror)
            {
                STOC_HS_PlayerChange scpc;
                scpc.status = (dp->type << 4) | PLAYERCHANGE_NOTREADY;
                NetServer::SendPacketToPlayer(dp, STOC_HS_PLAYER_CHANGE, scpc);
                STOC_ErrorMsg scem;
                scem.msg = ERRMSG_DECKERROR;
                scem.code = deckerror;
                NetServer::SendPacketToPlayer(dp, STOC_ERROR_MSG, scem);
                return;
            }
        }
        ready[dp->type] = is_ready;
        STOC_HS_PlayerChange scpc;
        scpc.status = (dp->type << 4) | (is_ready ? PLAYERCHANGE_READY : PLAYERCHANGE_NOTREADY);
        for (int i = 0; i < 4; ++i)
            if (players[i])
                NetServer::SendPacketToPlayer(players[i], STOC_HS_PLAYER_CHANGE, scpc);
        for (auto pit = observers.begin(); pit != observers.end(); ++pit)
            NetServer::SendPacketToPlayer(*pit, STOC_HS_PLAYER_CHANGE, scpc);
#ifdef YGOPRO_SERVER_MODE
        if (cache_recorder)
            NetServer::SendPacketToPlayer(cache_recorder, STOC_HS_PLAYER_CHANGE, scpc);
        if (replay_recorder)
            NetServer::SendPacketToPlayer(replay_recorder, STOC_HS_PLAYER_CHANGE, scpc);
#endif
    }
    void TagDuel::PlayerKick(DuelPlayer *dp, unsigned char pos)
    {
        if (pos > 3 || dp != host_player || dp == players[pos] || !players[pos])
            return;
        LeaveGame(players[pos]);
    }
    void TagDuel::UpdateDeck(DuelPlayer *dp, unsigned char *pdata, unsigned int len)
    {
        if (dp->type > 3 || ready[dp->type])
            return;
        if (len < sizeof(uint32_t) * 2)
            return;
        bool valid = true;
        uint32_t mainc = BufferIO::Read<uint32_t>(pdata);
        uint32_t sidec = BufferIO::Read<uint32_t>(pdata);
        if (mainc > MAINC_MAX)
            valid = false;
        else if (sidec > SIDEC_MAX)
            valid = false;
        else if (len < (2 + mainc + sidec) * sizeof(uint32_t))
            valid = false;
        if (!valid)
        {
#ifdef YGOPRO_SERVER_MODE
            STOC_HS_PlayerChange scpc;
            scpc.status = (dp->type << 4) | PLAYERCHANGE_NOTREADY;
            NetServer::SendPacketToPlayer(dp, STOC_HS_PLAYER_CHANGE, scpc);
#endif
            STOC_ErrorMsg scem;
            scem.msg = ERRMSG_DECKERROR;
            scem.code = 0;
            NetServer::SendPacketToPlayer(dp, STOC_ERROR_MSG, scem);
            return;
        }
        uint32_t deckbuf[MAINC_MAX + SIDEC_MAX];
        std::memcpy(deckbuf, pdata, (mainc + sidec) * sizeof(uint32_t));
        deck_error[dp->type] = DeckManager::LoadDeck(pdeck[dp->type], deckbuf, mainc, sidec);
#ifdef YGOPRO_SERVER_MODE
        PlayerReady(dp, true);
#endif
    }
    void TagDuel::StartDuel(DuelPlayer *dp)
    {
        if (dp != host_player)
            return;
        if (!ready[0] || !ready[1] || !ready[2] || !ready[3])
            return;
        NetServer::StopListen();
        // NetServer::StopBroadcast();
        for (int i = 0; i < 4; ++i)
            NetServer::SendPacketToPlayer(players[i], STOC_DUEL_START);
        for (auto oit = observers.begin(); oit != observers.end(); ++oit)
        {
            (*oit)->state = CTOS_LEAVE_GAME;
            NetServer::ReSendToPlayer(*oit);
        }
#ifdef YGOPRO_SERVER_MODE
        if (cache_recorder)
            cache_recorder->state = CTOS_LEAVE_GAME;
        if (replay_recorder)
            replay_recorder->state = CTOS_LEAVE_GAME;
        NetServer::ReSendToPlayers(cache_recorder, replay_recorder);
#endif
        unsigned char deckbuff[12];
        auto pbuf = deckbuff;
        BufferIO::Write<uint16_t>(pbuf, (uint16_t)pdeck[0].main.size());
        BufferIO::Write<uint16_t>(pbuf, (uint16_t)pdeck[0].extra.size());
        BufferIO::Write<uint16_t>(pbuf, (uint16_t)pdeck[0].side.size());
        BufferIO::Write<uint16_t>(pbuf, (uint16_t)pdeck[2].main.size());
        BufferIO::Write<uint16_t>(pbuf, (uint16_t)pdeck[2].extra.size());
        BufferIO::Write<uint16_t>(pbuf, (uint16_t)pdeck[2].side.size());
        NetServer::SendBufferToPlayer(players[0], STOC_DECK_COUNT, deckbuff, 12);
        NetServer::ReSendToPlayer(players[1]);
        char tempbuff[6];
        std::memcpy(tempbuff, deckbuff, 6);
        std::memcpy(deckbuff, deckbuff + 6, 6);
        std::memcpy(deckbuff + 6, tempbuff, 6);
        NetServer::SendBufferToPlayer(players[2], STOC_DECK_COUNT, deckbuff, 12);
        NetServer::ReSendToPlayer(players[3]);
        NetServer::SendPacketToPlayer(players[0], STOC_SELECT_HAND);
        NetServer::ReSendToPlayer(players[2]);
        hand_result[0] = 0;
        hand_result[1] = 0;
        players[0]->state = CTOS_HAND_RESULT;
        players[2]->state = CTOS_HAND_RESULT;
        duel_stage = DUEL_STAGE_FINGER;
    }
    void TagDuel::HandResult(DuelPlayer *dp, unsigned char res)
    {
        if (res == 0 || res > 3 || dp->state != CTOS_HAND_RESULT)
            return;
        auto player = (dp->type & 0x2) >> 1;
        if (hand_result[player])
            return;
        hand_result[player] = res;
        if (hand_result[0] && hand_result[1])
        {
            STOC_HandResult schr;
            schr.res1 = hand_result[0];
            schr.res2 = hand_result[1];
            NetServer::SendPacketToPlayer(players[0], STOC_HAND_RESULT, schr);
            NetServer::ReSendToPlayer(players[1]);
            for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                NetServer::ReSendToPlayer(*oit);
#ifdef YGOPRO_SERVER_MODE
            NetServer::ReSendToPlayers(cache_recorder, replay_recorder);
#endif
            schr.res1 = hand_result[1];
            schr.res2 = hand_result[0];
            NetServer::SendPacketToPlayer(players[2], STOC_HAND_RESULT, schr);
            NetServer::ReSendToPlayer(players[3]);
            if (hand_result[0] == hand_result[1])
            {
                NetServer::SendPacketToPlayer(players[0], STOC_SELECT_HAND);
                NetServer::ReSendToPlayer(players[2]);
                hand_result[0] = 0;
                hand_result[1] = 0;
                players[0]->state = CTOS_HAND_RESULT;
                players[2]->state = CTOS_HAND_RESULT;
            }
            else if ((hand_result[0] == 1 && hand_result[1] == 2) || (hand_result[0] == 2 && hand_result[1] == 3) || (hand_result[0] == 3 && hand_result[1] == 1))
            {
                NetServer::SendPacketToPlayer(players[2], STOC_SELECT_TP);
                players[0]->state = 0xff;
                players[2]->state = CTOS_TP_RESULT;
                duel_stage = DUEL_STAGE_FIRSTGO;
            }
            else
            {
                NetServer::SendPacketToPlayer(players[0], STOC_SELECT_TP);
                players[2]->state = 0xff;
                players[0]->state = CTOS_TP_RESULT;
                duel_stage = DUEL_STAGE_FIRSTGO;
            }
        }
    }
    void TagDuel::TPResult(DuelPlayer *dp, unsigned char tp)
    {
        if (dp->state != CTOS_TP_RESULT)
            return;
        duel_stage = DUEL_STAGE_DUELING;
        bool swapped = false;
        pplayer[0] = players[0];
        pplayer[1] = players[1];
        pplayer[2] = players[2];
        pplayer[3] = players[3];
        if ((tp && dp->type == 2) || (!tp && dp->type == 0))
        {
            std::swap(players[0], players[2]);
            std::swap(players[1], players[3]);
            players[0]->type = 0;
            players[1]->type = 1;
            players[2]->type = 2;
            players[3]->type = 3;
            std::swap(pdeck[0], pdeck[2]);
            std::swap(pdeck[1], pdeck[3]);
            swapped = true;
        }
        turn_count = 0;
        cur_player[0] = players[0];
        cur_player[1] = players[3];
        dp->state = CTOS_RESPONSE;
        std::random_device rd;
        ExtendedReplayHeader rh;
        rh.base.id = REPLAY_ID_YRP2;
        rh.base.version = PRO_VERSION;
        rh.base.flag = REPLAY_UNIFORM | REPLAY_TAG;
        rh.base.start_time = (uint32_t)std::time(nullptr);
#ifdef YGOPRO_SERVER_MODE
        if (pre_seed_specified[0])
            memcpy(rh.seed_sequence, pre_seed[0], SEED_COUNT * sizeof(uint32_t));
        else
#endif
            for (auto &x : rh.seed_sequence)
                x = rd();
        mtrandom rnd(rh.seed_sequence, SEED_COUNT);
#ifdef YGOPRO_SERVER_MODE
        pairing_random.seed(rh.seed_sequence, SEED_COUNT);
#endif
        last_replay.BeginRecord();
        last_replay.WriteHeader(rh);
        last_replay.WriteData(players[0]->name, 40, false);
        last_replay.WriteData(players[1]->name, 40, false);
        last_replay.WriteData(players[2]->name, 40, false);
        last_replay.WriteData(players[3]->name, 40, false);
        if (!host_info.no_shuffle_deck)
        {
            rnd.shuffle_vector(pdeck[0].main);
            rnd.shuffle_vector(pdeck[1].main);
            rnd.shuffle_vector(pdeck[2].main);
            rnd.shuffle_vector(pdeck[3].main);
        }
        time_limit[0] = host_info.time_limit;
        time_limit[1] = host_info.time_limit;
        set_script_reader(DataManager::ScriptReaderEx);
        set_card_reader(DataManager::CardReader);
        set_random_card_reader(DataManager::RandomCardReader);
        set_message_handler(TagDuel::MessageHandler);
        pduel = create_duel_v2(rh.seed_sequence);
#ifdef YGOPRO_SERVER_MODE
        preload_script(pduel, "./script/special.lua");
#endif
        set_player_info(pduel, 0, host_info.start_lp, host_info.start_hand, host_info.draw_count);
        set_player_info(pduel, 1, host_info.start_lp, host_info.start_hand, host_info.draw_count);
        unsigned int opt = (unsigned int)host_info.duel_rule << 16;
        if (host_info.no_shuffle_deck)
            opt |= DUEL_PSEUDO_SHUFFLE;
#ifndef YGOPRO_SERVER_MODE
        opt |= DUEL_TAG_MODE;
#endif
        last_replay.WriteInt32(host_info.start_lp, false);
        last_replay.WriteInt32(host_info.start_hand, false);
        last_replay.WriteInt32(host_info.draw_count, false);
        last_replay.WriteInt32(opt, false);
        last_replay.Flush();
#ifdef YGOPRO_SERVER_MODE
        auto load_home = [&](const std::vector<const CardDataC *> &deck_container, uint8_t field_id, uint8_t location)
        {
            last_replay.WriteInt32(deck_container.size(), false);
            for (auto cit = deck_container.rbegin(); cit != deck_container.rend(); ++cit)
            {
                new_card(pduel, (*cit)->code, 0, 0, location, 0, POS_FACEDOWN_DEFENSE, field_id);
                last_replay.WriteInt32((*cit)->code, false);
            }
        };
        for (uint8_t field_id = 0; field_id < HOME_FIELD_COUNT; ++field_id)
        {
            set_active_field(pduel, field_id);
            load_home(pdeck[field_id].main, field_id, LOCATION_DECK);
            load_home(pdeck[field_id].extra, field_id, LOCATION_EXTRA);
        }
#else
        auto load_single = [&](const std::vector<const CardDataC *> &deck_container, uint8_t p, uint8_t location)
        {
            last_replay.WriteInt32(deck_container.size(), false);
            for (auto cit = deck_container.rbegin(); cit != deck_container.rend(); ++cit)
            {
                new_card(pduel, (*cit)->code, p, p, location, 0, POS_FACEDOWN_DEFENSE, 0);
                last_replay.WriteInt32((*cit)->code, false);
            }
        };
        auto load_tag = [&](const std::vector<const CardDataC *> &deck_container, uint8_t p, uint8_t location)
        {
            last_replay.WriteInt32(deck_container.size(), false);
            for (auto cit = deck_container.rbegin(); cit != deck_container.rend(); ++cit)
            {
                new_tag_card(pduel, (*cit)->code, p, location, 0);
                last_replay.WriteInt32((*cit)->code, false);
            }
        };
        load_single(pdeck[0].main, 0, LOCATION_DECK);
        load_single(pdeck[0].extra, 0, LOCATION_EXTRA);
        load_tag(pdeck[1].main, 0, LOCATION_DECK);
        load_tag(pdeck[1].extra, 0, LOCATION_EXTRA);
        load_single(pdeck[3].main, 1, LOCATION_DECK);
        load_single(pdeck[3].extra, 1, LOCATION_EXTRA);
        load_tag(pdeck[2].main, 1, LOCATION_DECK);
        load_tag(pdeck[2].extra, 1, LOCATION_EXTRA);
#endif
        last_replay.Flush();
#ifndef YGOPRO_SERVER_MODE
        unsigned char startbuf[32]{};
        auto pbuf = startbuf;
        BufferIO::Write<uint8_t>(pbuf, MSG_START);
        BufferIO::Write<uint8_t>(pbuf, 0);
        BufferIO::Write<uint8_t>(pbuf, host_info.duel_rule);
        BufferIO::Write<int32_t>(pbuf, host_info.start_lp);
        BufferIO::Write<int32_t>(pbuf, host_info.start_lp);
        BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, 0, 0, LOCATION_DECK));
        BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, 0, 0, LOCATION_EXTRA));
        BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, 0, 1, LOCATION_DECK));
        BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, 0, 1, LOCATION_EXTRA));
        NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, startbuf, 19);
        NetServer::ReSendToPlayer(players[1]);
        startbuf[1] = 1;
        NetServer::SendBufferToPlayer(players[2], STOC_GAME_MSG, startbuf, 19);
        NetServer::ReSendToPlayer(players[3]);
        if (!swapped)
            startbuf[1] = 0x10;
        else
            startbuf[1] = 0x11;
        for (auto oit = observers.begin(); oit != observers.end(); ++oit)
            NetServer::SendBufferToPlayer(*oit, STOC_GAME_MSG, startbuf, 19);
        RefreshExtra(0);
        RefreshExtra(1);
#else
        turn_player = 0;
        phase = 1;
        deck_reversed = false;
#endif
        start_duel(pduel, opt);
        if (host_info.time_limit)
        {
            time_elapsed = 0;
#ifdef YGOPRO_SERVER_MODE
            time_compensator[0] = host_info.time_limit;
            time_compensator[1] = host_info.time_limit;
            time_backed[0] = host_info.time_limit;
            time_backed[1] = host_info.time_limit;
            last_game_msg = 0;
#endif
            timeval timeout = {1, 0};
            event_add(etimer, &timeout);
        }
#ifdef YGOPRO_SERVER_MODE
        for (uint8_t field_id = 0; field_id < HOME_FIELD_COUNT; ++field_id)
        {
            active_field = field_id;
            set_active_field(pduel, active_field);
            fprintf(stderr, "[DEBUG] TPDuel::Process: before Process() field %d\n", (int)active_field);
            fflush(stderr);
            Process();
        }
#else
        Process();
#endif
    }
    void TagDuel::Process()
    {
        std::vector<unsigned char> engineBuffer;
        engineBuffer.reserve(SIZE_MESSAGE_BUFFER);
        unsigned int engFlag = 0;
        int engLen = 0;
        int stop = 0;
        while (!stop)
        {
            if (engFlag == PROCESSOR_END)
                break;
            unsigned int result = process(pduel);
            engLen = result & PROCESSOR_BUFFER_LEN;
            engFlag = result & PROCESSOR_FLAG;
            if (engLen > 0)
            {
                if (engLen > (int)engineBuffer.size())
                    engineBuffer.resize(engLen);
                fprintf(stderr, "[DEBUG] before get_message engLen=%d\n", engLen);
                fflush(stderr);
                get_message(pduel, engineBuffer.data());
                fprintf(stderr, "[DEBUG] after get_message, msgType=0x%x, before Analyze\n", (unsigned)engineBuffer[0]);
                fflush(stderr);
                stop = Analyze(engineBuffer.data(), engLen);
                fprintf(stderr, "[DEBUG] after Analyze, stop=%d\n", stop);
                fflush(stderr);
            }
        }
        if (stop == 2)
            DuelEndProc();
    }
    void TagDuel::DuelEndProc()
    {
        NetServer::SendPacketToPlayer(players[0], STOC_DUEL_END);
        NetServer::ReSendToPlayer(players[1]);
        NetServer::ReSendToPlayer(players[2]);
        NetServer::ReSendToPlayer(players[3]);
        for (auto oit = observers.begin(); oit != observers.end(); ++oit)
            NetServer::ReSendToPlayer(*oit);
#ifdef YGOPRO_SERVER_MODE
        NetServer::ReSendToPlayers(cache_recorder, replay_recorder);
        NetServer::StopServer();
#else
        duel_stage = DUEL_STAGE_END;
#endif
    }
    void TagDuel::Surrender(DuelPlayer *dp)
    {
        if (dp->type > 3 || !pduel || eliminated_players[dp->type] == true)
            return;
        int remaining_players = 0;
#ifdef YGOPRO_SERVER_MODE
        for (int i = 0; i < HOME_FIELD_COUNT; ++i)
        {
            if (!eliminated_players[i])
                remaining_players++;
        }
#endif
        uint32_t player = dp->type;
        if (remaining_players <= 2)
        {
#if !defined(YGOPRO_SERVER_MODE) || defined(SERVER_TAG_SURRENDER_CONFIRM)
            if (surrender[player])
                return;
            static const uint32_t teammatemap[] = {1, 0, 3, 2};
            uint32_t teammate = teammatemap[player];
            if (!surrender[teammate])
            {
                surrender[player] = true;
                NetServer::SendPacketToPlayer(players[player], STOC_TEAMMATE_SURRENDER);
                NetServer::SendPacketToPlayer(players[teammate], STOC_TEAMMATE_SURRENDER);
                return;
            }
#endif
            static const uint32_t winplayermap[] = {1, 1, 0, 0};
            unsigned char wbuf[3];
            wbuf[0] = MSG_WIN;
            wbuf[1] = winplayermap[player];
            wbuf[2] = 0;
            NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, wbuf, 3);
            NetServer::ReSendToPlayer(players[1]);
            NetServer::ReSendToPlayer(players[2]);
            NetServer::ReSendToPlayer(players[3]);
            for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                NetServer::ReSendToPlayer(*oit);
            EndDuel();
            DuelEndProc();
            event_del(etimer);
        }
        else
        {
#ifdef YGOPRO_SERVER_MODE
            int engine_playerid = 0;
            if (active_field >= HOME_FIELD_COUNT)
            {
                for (int i = 0; i < active_battle_count; ++i)
                {
                    if (battle_pairs[i][0] == player)
                    {
                        engine_playerid = 0;
                        break;
                    }
                    if (battle_pairs[i][1] == player)
                    {
                        engine_playerid = 1;
                        break;
                    }
                }
            }
            fprintf(stderr, "[DEBUG] 有人投降了 玩家id=%d 所属场地的玩家id=%d\n", player, engine_playerid);
            fflush(stderr);
            unsigned char wbuf[3];
            wbuf[0] = MSG_WIN;
            wbuf[1] = 1 - engine_playerid;
            wbuf[2] = 0;
            NetServer::SendBufferToPlayer(dp, STOC_GAME_MSG, wbuf, 3);

            eliminated_players[player] = true;
            set_player_corpse(pduel, active_field, engine_playerid);

            std::wstring obs_msg = std::wstring(reinterpret_cast<const wchar_t *>(dp->name)) + dataManager.GetSysString(1801);
            for (int i = 0; i < HOME_FIELD_COUNT; ++i)
            {
                if (i != player)
                {
                    send_system_chat(players[i], obs_msg.c_str());
                }
            }
            for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                send_system_chat(*oit, obs_msg.c_str());

            if (dp->state == CTOS_RESPONSE)
            {
                Process();
            }
#endif
        }
    }
    int TagDuel::Analyze(unsigned char *msgbuffer, unsigned int len)
    {
        unsigned char *offset, *pbufw, *pbuf = msgbuffer;
        int player, count, type;
        while (pbuf - msgbuffer < (int)len)
        {
            offset = pbuf;
            unsigned char engType = BufferIO::Read<uint8_t>(pbuf);
#ifdef YGOPRO_SERVER_MODE
            last_game_msg = engType;
#endif
            fprintf(stderr, "[DEBUG] Analyze: engType=%d\n", (int)engType);
            fflush(stderr);
            switch (engType)
            {
#ifdef YGOPRO_SERVER_MODE
            case MSG_FIELD_READY:
            {
                const auto turn_player_from_engine = BufferIO::Read<uint8_t>(pbuf);
                const auto lp0 = BufferIO::Read<uint8_t>(pbuf);
                const auto lp1 = BufferIO::Read<uint8_t>(pbuf);
                const uint8_t ready_field = active_field;
                // engine 信号：field 初始化完成，构建并发送 MSG_START
                unsigned char startbuf[32]{};
                auto pbuf = startbuf;
                BufferIO::Write<uint8_t>(pbuf, MSG_START);
                BufferIO::Write<uint8_t>(pbuf, 0);
                BufferIO::Write<uint8_t>(pbuf, host_info.duel_rule);
                BufferIO::Write<int32_t>(pbuf, lp0);
                BufferIO::Write<int32_t>(pbuf, lp1);
                if (!IsBattleField())
                {
                    // 非战斗场：只发本 field 家园主，对手数据填 0
                    BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, ready_field, 0, LOCATION_DECK));
                    BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, ready_field, 0, LOCATION_EXTRA));
                    BufferIO::Write<uint16_t>(pbuf, 0);
                    BufferIO::Write<uint16_t>(pbuf, 0);
                    NetServer::SendBufferToPlayer(players[ready_field], STOC_GAME_MSG, startbuf, 19);
                }
                else
                {
                    // 战斗场：使用 engine 计算的 turn_player，发给 battle_pairs 中的双方
                    const unsigned char battle_id = ready_field - HOME_FIELD_COUNT;
                    BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, ready_field, 0, LOCATION_DECK));
                    BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, ready_field, 0, LOCATION_EXTRA));
                    BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, ready_field, 1, LOCATION_DECK));
                    BufferIO::Write<uint16_t>(pbuf, query_field_count(pduel, ready_field, 1, LOCATION_EXTRA));
                    startbuf[1] = turn_player_from_engine;
                    NetServer::SendBufferToPlayer(players[battle_pairs[battle_id][turn_player_from_engine]], STOC_GAME_MSG, startbuf, 19);
                    startbuf[1] = 1 - turn_player_from_engine;
                    NetServer::SendBufferToPlayer(players[battle_pairs[battle_id][1 - turn_player_from_engine]], STOC_GAME_MSG, startbuf, 19);
                }
                // observers/recorders 始终发送
                for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                    NetServer::SendBufferToPlayer(*oit, STOC_GAME_MSG, startbuf, 19);
                if (cache_recorder)
                    NetServer::SendBufferToPlayer(cache_recorder, STOC_GAME_MSG, startbuf, 19);
                if (replay_recorder)
                    NetServer::SendBufferToPlayer(replay_recorder, STOC_GAME_MSG, startbuf, 19);
                // 全量刷新：参考 RequestField 刷新双方所有区域
                {
                    uint8_t refresh_buf[1024];
                    auto write_refresh_msg = [&](const std::function<void(uint8_t *&)> &writer, DuelPlayer *target)
                    {
                        uint8_t *pbuf = refresh_buf;
                        writer(pbuf);
                        NetServer::SendBufferToPlayer(target, STOC_GAME_MSG, refresh_buf, pbuf - refresh_buf);
                    };
                    auto send_deck_top = [&](uint8_t field, DuelPlayer *target)
                    {
                        uint8_t query_buffer[SIZE_QUERY_BUFFER];
                        for (uint8_t i = 0; i < 2; ++i)
                        {
                            auto qlen = query_field_card(pduel, field, i, LOCATION_DECK, QUERY_CODE | QUERY_POSITION, query_buffer, 0);
                            if (!qlen)
                                continue;
                            uint8_t *qbuf = query_buffer;
                            uint32_t code = 0;
                            uint32_t position = 0;
                            while (qbuf < query_buffer + qlen)
                            {
                                auto clen = BufferIO::Read<int32_t>(qbuf);
                                if (qbuf + clen - 4 == query_buffer + qlen)
                                {
                                    code = *(uint32_t *)(qbuf + 4);
                                    position = GetPosition(qbuf, 8);
                                }
                                qbuf += clen - 4;
                            }
                            if (position & POS_FACEUP)
                                code |= 0x80000000;
                            if (deck_reversed || position & POS_FACEUP)
                                write_refresh_msg([&](uint8_t *&wpbuf)
                                                  {
                                    BufferIO::Write<uint8_t>(wpbuf, MSG_DECK_TOP);
                                    BufferIO::Write<uint8_t>(wpbuf, i);
                                    BufferIO::Write<uint8_t>(wpbuf, 0);
                                    BufferIO::Write<int32_t>(wpbuf, code); }, target);
                        }
                    };
                    if (!IsBattleField())
                    {
                        // 非战斗场：刷新双方数据，只发给本 field 家园主
                        auto target = players[ready_field];
                        // query_field_info
                        {
                            uint8_t qfi_buf[SIZE_QUERY_BUFFER];
                            auto len = query_field_info(pduel, ready_field, qfi_buf);
                            NetServer::SendBufferToPlayer(target, STOC_GAME_MSG, qfi_buf, len);
                        }
                        RefreshMzone(0, 0xefffff, 0, ready_field);
                        RefreshMzone(1, 0xefffff, 0, ready_field);
                        RefreshSzone(0, 0xefffff, 0, ready_field);
                        RefreshSzone(1, 0xefffff, 0, ready_field);
                        RefreshHand(0, 0xefffff, 0, ready_field);
                        RefreshHand(1, 0xefffff, 0, ready_field);
                        RefreshGrave(0, 0xefffff, 0, ready_field);
                        RefreshGrave(1, 0xefffff, 0, ready_field);
                        RefreshExtra(0, 0xefffff, 0, ready_field);
                        RefreshExtra(1, 0xefffff, 0, ready_field);
                        RefreshRemoved(0, 0xefffff, 0, ready_field);
                        RefreshRemoved(1, 0xefffff, 0, ready_field);
                        if (deck_reversed)
                            write_refresh_msg([&](uint8_t *&wpbuf)
                                              { BufferIO::Write<uint8_t>(wpbuf, MSG_REVERSE_DECK); }, target);
                        send_deck_top(ready_field, target);
                    }
                    else
                    {
                        // 战斗场：分别发给 battle_pairs 中的双方玩家
                        const unsigned char battle_id = ready_field - HOME_FIELD_COUNT;
                        for (uint8_t p = 0; p < 2; ++p)
                        {
                            auto target = players[battle_pairs[battle_id][p]];
                            // query_field_info
                            {
                                uint8_t qfi_buf[SIZE_QUERY_BUFFER];
                                auto len = query_field_info(pduel, ready_field, qfi_buf);
                                NetServer::SendBufferToPlayer(target, STOC_GAME_MSG, qfi_buf, len);
                            }
                            RefreshMzone(0, 0xefffff, 0, ready_field, target);
                            RefreshMzone(1, 0xefffff, 0, ready_field, target);
                            RefreshSzone(0, 0xefffff, 0, ready_field, target);
                            RefreshSzone(1, 0xefffff, 0, ready_field, target);
                            RefreshHand(0, 0xefffff, 0, ready_field, target);
                            RefreshHand(1, 0xefffff, 0, ready_field, target);
                            RefreshGrave(0, 0xefffff, 0, ready_field, target);
                            RefreshGrave(1, 0xefffff, 0, ready_field, target);
                            RefreshExtra(0, 0xefffff, 0, ready_field, target);
                            RefreshExtra(1, 0xefffff, 0, ready_field, target);
                            RefreshRemoved(0, 0xefffff, 0, ready_field, target);
                            RefreshRemoved(1, 0xefffff, 0, ready_field, target);
                            if (deck_reversed)
                                write_refresh_msg([&](uint8_t *&wpbuf)
                                                  { BufferIO::Write<uint8_t>(wpbuf, MSG_REVERSE_DECK); }, target);
                            send_deck_top(ready_field, target);
                        }
                    }
                }
                break;
            }
            case MSG_CHANGE_FIELD:
            {
                if (active_field < HOME_FIELD_COUNT)
                {
                    if (eliminated_players[active_field])
                        return 1;
                    home_battle_ready[active_field] = true;
                    unsigned char alive_count = 0;
                    unsigned char ready_count = 0;
                    for (unsigned char field_id = 0; field_id < HOME_FIELD_COUNT; ++field_id)
                    {
                        if (eliminated_players[field_id])
                            continue;
                        ++alive_count;
                        if (home_battle_ready[field_id])
                            ++ready_count;
                    }
                    if (ready_count < alive_count)
                        return 1;
                    if (alive_count < 2)
                    {
                        AbortDuelWithChat(u"Not enough living players for battle; duel terminated.");
                        return 2;
                    }

                    if (alive_count == 2)
                    {
                        unsigned char pair_index = 0;
                        for (unsigned char player_id = 0; player_id < HOME_FIELD_COUNT; ++player_id)
                        {
                            if (!eliminated_players[player_id])
                                battle_pairs[0][pair_index++] = player_id;
                        }
                        previous_opponent[battle_pairs[0][0]] = battle_pairs[0][1];
                        previous_opponent[battle_pairs[0][1]] = battle_pairs[0][0];
                        active_battle_count = 1;
                    }
                    else
                    {
                        static const unsigned char complete_pairings[3][BATTLE_FIELD_COUNT][2] = {
                            {{0, 1}, {2, 3}},
                            {{0, 2}, {1, 3}},
                            {{0, 3}, {1, 2}}};
                        unsigned char legal_pairings[3]{};
                        unsigned char legal_count = 0;
                        for (unsigned char pairing_id = 0; pairing_id < 3; ++pairing_id)
                        {
                            bool legal = true;
                            for (unsigned char battle_id = 0; battle_id < BATTLE_FIELD_COUNT; ++battle_id)
                            {
                                const unsigned char first_player = complete_pairings[pairing_id][battle_id][0];
                                const unsigned char second_player = complete_pairings[pairing_id][battle_id][1];
                                if (previous_opponent[first_player] == second_player || previous_opponent[second_player] == first_player)
                                {
                                    legal = false;
                                    break;
                                }
                            }
                            if (legal)
                                legal_pairings[legal_count++] = pairing_id;
                        }
                        if (!legal_count)
                        {
                            AbortDuelWithChat(u"No legal complete pairing; duel terminated.");
                            return 2;
                        }
                        const unsigned char selected = legal_pairings[pairing_random.get_random_integer_v2(0, legal_count - 1)];
                        for (unsigned char battle_id = 0; battle_id < BATTLE_FIELD_COUNT; ++battle_id)
                        {
                            battle_pairs[battle_id][0] = complete_pairings[selected][battle_id][0];
                            fprintf(stderr, "[DEBUG] 战斗配队场地=%d 玩家0是=%d", battle_id, (int)battle_pairs[battle_id][0]);
                            fflush(stderr);
                            battle_pairs[battle_id][1] = complete_pairings[selected][battle_id][1];
                            fprintf(stderr, "[DEBUG] 战斗配队场地=%d 玩家1是=%d", battle_id, (int)battle_pairs[battle_id][1]);
                            fflush(stderr);
                            previous_opponent[battle_pairs[battle_id][0]] = battle_pairs[battle_id][1];
                            previous_opponent[battle_pairs[battle_id][1]] = battle_pairs[battle_id][0];
                        }
                        active_battle_count = BATTLE_FIELD_COUNT;
                    }

                    for (unsigned char field_id = 0; field_id < HOME_FIELD_COUNT; ++field_id)
                        home_battle_ready[field_id] = false;
                    for (unsigned char battle_id = 0; battle_id < BATTLE_FIELD_COUNT; ++battle_id)
                        battle_finished[battle_id] = false;
                    for (unsigned char battle_id = 0; battle_id < active_battle_count; ++battle_id)
                    {
                        const unsigned char battle_field = HOME_FIELD_COUNT + battle_id;
                        const unsigned char battle_turn_player = battle_turn_counter[battle_id] % 2;
                        set_active_field(pduel, battle_field);
                        if (merge_field_to_bp(pduel, battle_field, battle_pairs[battle_id][0], battle_pairs[battle_id][1], battle_turn_player) != TRUE)
                        {
                            AbortDuelWithChat(u"merge_field_to_bp failed; duel terminated.");
                            return 2;
                        }
                        ++battle_turn_counter[battle_id];
                    }
                    for (unsigned char battle_id = 0; battle_id < active_battle_count; ++battle_id)
                    {
                        const unsigned char battle_field = HOME_FIELD_COUNT + battle_id;
                        set_active_field(pduel, battle_field);
                        active_field = battle_field;
                        Process();
                    }
                    return 1;
                }

                if (active_field >= FIELD_COUNT)
                    return 1;
                const unsigned char battle_id = active_field - HOME_FIELD_COUNT;
                if (battle_id >= active_battle_count)
                    return 1;
                if (battle_finished[battle_id])
                    return 1;
                battle_finished[battle_id] = true;
                for (unsigned char active_id = 0; active_id < active_battle_count; ++active_id)
                {
                    if (!battle_finished[active_id])
                    {
                        send_battle_system_chat(battle_id, 1800);
                        return 1;
                    }
                }
                for (unsigned char active_id = 0; active_id < active_battle_count; ++active_id)
                {
                    const unsigned char battle_field = HOME_FIELD_COUNT + active_id;
                    set_active_field(pduel, battle_field);
                    active_field = battle_field;
                    if (return_field_to_main(pduel, battle_field) != TRUE)
                    {
                        AbortDuelWithChat(u"return_field_to_main failed; duel terminated.");
                        return 2;
                    }
                }
                active_battle_count = 0;
                for (unsigned char field_id = 0; field_id < HOME_FIELD_COUNT; ++field_id)
                {
                    if (eliminated_players[field_id])
                        continue;
                    set_active_field(pduel, field_id);
                    active_field = field_id;
                    Process();
                }
                return 1;
            }
#endif
            case MSG_RETRY:
            {
                if (last_replay_response_size)
                {
                    last_replay.RemoveData(last_replay_response_size);
                    last_replay_response_size = 0;
                }
                fprintf(stderr, "[DEBUG] MSG_RETRY: last_response=%d active_field=%d remapped=%d\n", (int)last_response, (int)active_field, remap_field_player(last_response));
                fflush(stderr);
                WaitforResponse(last_response);
                const int remapped = remap_field_player(last_response);
                if (remapped != -1)
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                return 1;
            }
            case MSG_HINT:
            {
                type = BufferIO::Read<uint8_t>(pbuf);
                player = BufferIO::Read<uint8_t>(pbuf);
                BufferIO::Read<int32_t>(pbuf);
                switch (type)
                {
                case 1:
                case 2:
                case 3:
                case 5:
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                    break;
                }
                case 4:
                case 6:
                case 7:
                case 8:
                case 9:
                case 11:
                {
#ifdef YGOPRO_SERVER_MODE
                    BcastObs(offset, pbuf - offset, true);
#endif
                    break;
                }
                case 10:
                {
#ifdef YGOPRO_SERVER_MODE
                    BcastObs(offset, pbuf - offset, false);
#endif
                    break;
                }
                }
                break;
            }
#ifdef YGOPRO_SERVER_MODE
            case MSG_CUSTOM_CHAT:
            {
                const size_t remaining = len - static_cast<size_t>(pbuf - msgbuffer);
                if (remaining < 2)
                    return -1;
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                if (player > 1 || count == 0 || remaining - 2 < static_cast<size_t>(count) * sizeof(uint16_t))
                    return -1;
                std::wstring message;
                for (int i = 0; i < count; ++i)
                {
                    const auto id = BufferIO::Read<uint16_t>(pbuf);
                    message.append(dataManager.GetSysString(id));
                }
                if (active_field >= FIELD_COUNT)
                    break;
                if (active_field < HOME_FIELD_COUNT)
                {
                    if (player != 0)
                        break;
                }
                else if (active_field - HOME_FIELD_COUNT >= active_battle_count)
                    break;
                {
                    const int remapped = remap_field_player(player);
                    if (remapped != -1)
                        send_system_chat(players[remapped], message.c_str());
                }
                break;
            }
#endif
            case MSG_WIN:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                type = BufferIO::Read<uint8_t>(pbuf);
#ifdef YGOPRO_SERVER_MODE
                if (active_battle_count > 1)
                {
                    if (IsBattleField())
                    {
                        const unsigned char battle_id = active_field - HOME_FIELD_COUNT;
                        if (battle_id >= active_battle_count)
                            return 1;
                        if (player != PLAYER_NONE)
                        {
                            const unsigned char loser = 1 - player;
                            eliminated_players[battle_pairs[battle_id][loser]] = true;
                            set_player_corpse(pduel, active_field, loser);
                            // 只发原始 win 给输家
                            NetServer::SendBufferToPlayer(players[battle_pairs[battle_id][loser]], STOC_GAME_MSG, offset, pbuf - offset);
                            // 赢家发系统消息：对方 + 已被淘汰提示
                            std::wstring winner_msg = std::wstring(dataManager.GetSysString(103)) + dataManager.GetSysString(1801);
                            send_system_chat(players[battle_pairs[battle_id][player]], winner_msg.c_str());
                            // 观战者发：输家名字 + 已被淘汰提示
                            std::wstring obs_msg = std::wstring(reinterpret_cast<const wchar_t *>(players[battle_pairs[battle_id][loser]]->name)) + dataManager.GetSysString(1801);
                            // 给其他玩家发淘汰信息
                            for (auto i = 0; i < active_battle_count; ++i)
                            {
                                if (i != battle_id)
                                {
                                    send_system_chat(players[battle_pairs[i][0]], obs_msg.c_str());
                                    send_system_chat(players[battle_pairs[i][1]], obs_msg.c_str());
                                }
                            }
                            for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                                send_system_chat(*oit, obs_msg.c_str());
                        }
                        else
                        {
                            // 双方同死：都标记淘汰，都发原始 win
                            eliminated_players[battle_pairs[battle_id][0]] = true;
                            eliminated_players[battle_pairs[battle_id][1]] = true;
                            set_player_corpse(pduel, active_field, 0);
                            set_player_corpse(pduel, active_field, 1);
                            NetServer::SendBufferToPlayer(players[battle_pairs[battle_id][0]], STOC_GAME_MSG, offset, pbuf - offset);
                            NetServer::SendBufferToPlayer(players[battle_pairs[battle_id][1]], STOC_GAME_MSG, offset, pbuf - offset);
                            std::wstring obs0 = std::wstring(reinterpret_cast<const wchar_t *>(players[battle_pairs[battle_id][0]]->name)) + dataManager.GetSysString(1801);
                            std::wstring obs1 = std::wstring(reinterpret_cast<const wchar_t *>(players[battle_pairs[battle_id][1]]->name)) + dataManager.GetSysString(1801);
                            // 给其他玩家发淘汰信息
                            for (auto i = 0; i < active_battle_count; ++i)
                            {
                                if (i != battle_id)
                                {
                                    send_system_chat(players[battle_pairs[i][0]], obs0.c_str());
                                    send_system_chat(players[battle_pairs[i][0]], obs1.c_str());
                                    send_system_chat(players[battle_pairs[i][1]], obs0.c_str());
                                    send_system_chat(players[battle_pairs[i][1]], obs1.c_str());
                                }
                            }
                            for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                            {
                                send_system_chat(*oit, obs0.c_str());
                                send_system_chat(*oit, obs1.c_str());
                            }
                        }
                    }
                    else
                    {
                        // 家园场收到 MSG_WIN（防御）：标记主人淘汰
                        eliminated_players[active_field] = true;
                        // 设置主人为尸体
                        set_player_corpse(pduel, active_field, 0);
                        NetServer::SendBufferToPlayer(players[active_field], STOC_GAME_MSG, offset, pbuf - offset);
                        std::wstring obs_msg = std::wstring(reinterpret_cast<const wchar_t *>(players[active_field]->name)) + dataManager.GetSysString(1801);
                        for (auto i = 0; i < HOME_FIELD_COUNT; ++i)
                        {
                            if (i == active_field || eliminated_players[i])
                                continue;
                            send_system_chat(players[i], obs_msg.c_str());
                        }
                        for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                            send_system_chat(*oit, obs_msg.c_str());
                    }
                    break;
                }
                BcastObs(offset, pbuf - offset, true);
#endif
                EndDuel();
                return 2;
            }
            case MSG_SELECT_BATTLECMD:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 11;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 8 + 2;
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                RefreshHand(0);
                RefreshHand(1);
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_IDLECMD:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 11 + 3;
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                RefreshHand(0);
                RefreshHand(1);
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_EFFECTYN:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 12;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_YESNO:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 4;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_OPTION:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 4;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_CARD:
            case MSG_SELECT_TRIBUTE:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 3;
                count = BufferIO::Read<uint8_t>(pbuf);
                int c /*, l, s, ss, code*/;
                for (int i = 0; i < count; ++i)
                {
                    pbufw = pbuf;
                    /*code = */ BufferIO::Read<int32_t>(pbuf);
                    c = BufferIO::Read<uint8_t>(pbuf);
                    /*l = */ BufferIO::Read<uint8_t>(pbuf);
                    /*s = */ BufferIO::Read<uint8_t>(pbuf);
                    /*ss = */ BufferIO::Read<uint8_t>(pbuf);
                    if (c != player)
                        BufferIO::Write<int32_t>(pbufw, 0);
                }
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_UNSELECT_CARD:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 4;
                count = BufferIO::Read<uint8_t>(pbuf);
                int c /*, l, s, ss, code*/;
                for (int i = 0; i < count; ++i)
                {
                    pbufw = pbuf;
                    /*code = */ BufferIO::Read<int32_t>(pbuf);
                    c = BufferIO::Read<uint8_t>(pbuf);
                    /*l = */ BufferIO::Read<uint8_t>(pbuf);
                    /*s = */ BufferIO::Read<uint8_t>(pbuf);
                    /*ss = */ BufferIO::Read<uint8_t>(pbuf);
                    if (c != player)
                        BufferIO::Write<int32_t>(pbufw, 0);
                }
                count = BufferIO::Read<uint8_t>(pbuf);
                for (int i = 0; i < count; ++i)
                {
                    pbufw = pbuf;
                    /*code = */ BufferIO::Read<int32_t>(pbuf);
                    c = BufferIO::Read<uint8_t>(pbuf);
                    /*l = */ BufferIO::Read<uint8_t>(pbuf);
                    /*s = */ BufferIO::Read<uint8_t>(pbuf);
                    /*ss = */ BufferIO::Read<uint8_t>(pbuf);
                    if (c != player)
                        BufferIO::Write<int32_t>(pbufw, 0);
                }
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_CHAIN:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 9 + count * 14;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_PLACE:
            case MSG_SELECT_DISFIELD:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 5;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_POSITION:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 5;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_COUNTER:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 4;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 9;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SELECT_SUM:
            {
                pbuf++;
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 6;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 11;
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 11;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_SORT_CARD:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_CONFIRM_DECKTOP:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CONFIRM_EXTRATOP:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 7;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CONFIRM_CARDS:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 1;
                count = BufferIO::Read<uint8_t>(pbuf);
                if (pbuf[5] != LOCATION_DECK)
                {
                    pbuf += count * 7;
#ifdef YGOPRO_SERVER_MODE
                    BcastObs(offset, pbuf - offset, true);
#endif
                }
                else
                {
                    pbuf += count * 7;
                    {
                        const int remapped = remap_field_player(last_response);
                        if (remapped != -1)
                            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                    }
                }
                break;
            }
            case MSG_SHUFFLE_DECK:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_SHUFFLE_HAND:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                const int remapped = remap_field_player(player);
                if (remapped != -1)
                {
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, (pbuf - offset) + count * 4);
#ifdef YGOPRO_SERVER_MODE
                    NetServer::ReSendToPlayer(replay_recorder);
#endif
                }
                for (int i = 0; i < count; ++i)
                    BufferIO::Write<int32_t>(pbuf, 0);
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true, remapped);
#endif
                RefreshHand(player, 0x781fff, 0);
                break;
            }
            case MSG_SHUFFLE_EXTRA:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                const int remapped = remap_field_player(player);
                if (remapped != -1)
                {
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, (pbuf - offset) + count * 4);
#ifdef YGOPRO_SERVER_MODE
                    NetServer::ReSendToPlayer(replay_recorder);
#endif
                }
                for (int i = 0; i < count; ++i)
                    BufferIO::Write<int32_t>(pbuf, 0);
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true, remapped);
#endif
                RefreshExtra(player);
                break;
            }
            case MSG_REFRESH_DECK:
            {
                pbuf++;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_SWAP_GRAVE_DECK:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshGrave(player);
                break;
            }
            case MSG_REVERSE_DECK:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
                deck_reversed = !deck_reversed;
#endif
                break;
            }
            case MSG_DECK_TOP:
            {
                pbuf += 6;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_SHUFFLE_SET_CARD:
            {
                unsigned int loc = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 8;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                if (loc == LOCATION_MZONE)
                {
                    RefreshMzone(0, 0x181fff, 0);
                    RefreshMzone(1, 0x181fff, 0);
                }
                else
                {
                    RefreshSzone(0, 0x181fff, 0);
                    RefreshSzone(1, 0x181fff, 0);
                }
                break;
            }
            case MSG_NEW_TURN:
            {
#ifdef YGOPRO_SERVER_MODE
                turn_player = BufferIO::Read<uint8_t>(pbuf);
#else
                pbuf++;
#endif
                time_limit[0] = host_info.time_limit;
                time_limit[1] = host_info.time_limit;
#ifdef YGOPRO_SERVER_MODE
                time_compensator[0] = host_info.time_limit;
                time_compensator[1] = host_info.time_limit;
                time_backed[0] = host_info.time_limit;
                time_backed[1] = host_info.time_limit;
#endif
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                if (turn_count > 0)
                {
                    if (turn_count % 2 == 0)
                    {
                        if (cur_player[0] == players[0])
                            cur_player[0] = players[1];
                        else
                            cur_player[0] = players[0];
                    }
                    else
                    {
                        if (cur_player[1] == players[2])
                            cur_player[1] = players[3];
                        else
                            cur_player[1] = players[2];
                    }
                }
                turn_count++;
                for (int i = 0; i < 4; ++i)
                {
                    surrender[i] = false;
                }
                break;
            }
            case MSG_NEW_PHASE:
            {
#ifdef YGOPRO_SERVER_MODE
                phase = BufferIO::Read<uint16_t>(pbuf);
#else
                pbuf += 2;
#endif
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                RefreshHand(0);
                RefreshHand(1);
                break;
            }
            case MSG_MOVE:
            {
                pbufw = pbuf;
                int pc = pbuf[4];
                int pl = pbuf[5];
                /*int ps = pbuf[6];*/
                /*int pp = pbuf[7];*/
                int cc = pbuf[8];
                int cl = pbuf[9];
                int cs = pbuf[10];
                uint8_t cp = pbuf[11];
                fprintf(stderr, "[DEBUG] MSG_MOVE: cc=%d pc=%d cl=%d pl=%d active_field=%d IsBattleField=%d\n", cc, pc, cl, pl, (int)active_field, (int)IsBattleField());
                fflush(stderr);
                const bool hide_code = NetServer::ShouldHideFacedownCode(cp);
                if (cl & LOCATION_ONFIELD)
                    cp = NetServer::StripRevealFlag(pbufw, 8);
                pbuf += 16;
                const int remapped = remap_field_player(cc);
                if (remapped != -1)
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                if (!(cl & (LOCATION_GRAVE | LOCATION_OVERLAY)) && ((cl & (LOCATION_DECK | LOCATION_HAND)) || hide_code))
                    BufferIO::Write<int32_t>(pbufw, 0);
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true, remapped);
#endif
                if (cl != 0 && (cl & LOCATION_OVERLAY) == 0 && (cl != pl || pc != cc))
                    RefreshSingle(cc, cl, cs);
                break;
            }
            case MSG_POS_CHANGE:
            {
                int cc = pbuf[4];
                int cl = pbuf[5];
                int cs = pbuf[6];
                int pp = pbuf[7];
                int cp = pbuf[8];
                pbuf += 9;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                if ((pp & POS_FACEDOWN) && (cp & POS_FACEUP))
                    RefreshSingle(cc, cl, cs);
                break;
            }
            case MSG_SET:
            {
                BufferIO::Write<int32_t>(pbuf, 0);
                pbuf += 4;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_SWAP:
            {
                int c1 = pbuf[4];
                int l1 = pbuf[5];
                int s1 = pbuf[6];
                int c2 = pbuf[12];
                int l2 = pbuf[13];
                int s2 = pbuf[14];
                pbuf += 16;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshSingle(c1, l1, s1);
                RefreshSingle(c2, l2, s2);
                break;
            }
            case MSG_FIELD_DISABLED:
            {
                pbuf += 4;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_SUMMONING:
            {
                pbuf += 8;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_SUMMONED:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                break;
            }
            case MSG_SPSUMMONING:
            {
                pbufw = pbuf;
                int cc = pbuf[4];
                /*int cl = pbuf[5];*/
                /*int cs = pbuf[6];*/
                uint8_t cp = pbuf[7];
                const bool hide_code = NetServer::ShouldHideFacedownCode(cp);
                cp = NetServer::StripRevealFlag(pbufw, 4);
                pbuf += 8;
                // auto pid = (cc == 0) ? 0 : 2;
                int remapped = remap_field_player(cc);
                if (remapped != -1)
                {
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
#ifdef YGOPRO_SERVER_MODE
                    NetServer::ReSendToPlayer(replay_recorder);
#endif
                }
                if (hide_code)
                    BufferIO::Write<int32_t>(pbufw, 0);
                remapped = remap_field_player(1 - cc);
                if (remapped != -1)
                {
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                    NetServer::ReSendToPlayer(*oit);
#ifdef YGOPRO_SERVER_MODE
                NetServer::ReSendToPlayer(cache_recorder);
#endif
                break;
            }
            case MSG_SPSUMMONED:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                break;
            }
            case MSG_FLIPSUMMONING:
            {
                RefreshSingle(pbuf[4], pbuf[5], pbuf[6]);
                pbuf += 8;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_FLIPSUMMONED:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                break;
            }
            case MSG_CHAINING:
            {
                pbuf += 16;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CHAINED:
            {
                pbuf++;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                RefreshHand(0);
                RefreshHand(1);
                break;
            }
            case MSG_CHAIN_SOLVING:
            {
                pbuf++;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CHAIN_SOLVED:
            {
                pbuf++;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                RefreshHand(0);
                RefreshHand(1);
                break;
            }
            case MSG_CHAIN_END:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                RefreshSzone(0);
                RefreshSzone(1);
                RefreshHand(0);
                RefreshHand(1);
                break;
            }
            case MSG_CHAIN_NEGATED:
            {
                pbuf++;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CHAIN_DISABLED:
            {
                pbuf++;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CARD_SELECTED:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 4;
                break;
            }
            case MSG_RANDOM_SELECTED:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 4;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_BECOME_TARGET:
            {
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count * 4;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_DRAW:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                fprintf(stderr, "[DEBUG] MSG_DRAW: engine_player=%d active_field=%d IsBattleField=%d\n", player, (int)active_field, (int)IsBattleField());
                fflush(stderr);
                pbufw = pbuf;
                pbuf += count * 4;
                const int remapped = remap_field_player(player);
                if (remapped != -1)
                {
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
#ifdef YGOPRO_SERVER_MODE
                    NetServer::ReSendToPlayer(replay_recorder);
#endif
                }
                for (int i = 0; i < count; ++i)
                {
                    if (!(pbufw[3] & 0x80))
                        BufferIO::Write<int32_t>(pbufw, 0);
                    else
                        pbufw += 4;
                }
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true, remapped);
#endif
                break;
            }
            case MSG_DAMAGE:
            {
                pbuf += 5;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_RECOVER:
            {
                pbuf += 5;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_EQUIP:
            {
                pbuf += 8;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_LPUPDATE:
            {
                pbuf += 5;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_UNEQUIP:
            {
                pbuf += 4;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CARD_TARGET:
            {
                pbuf += 8;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_CANCEL_TARGET:
            {
                pbuf += 8;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_PAY_LPCOST:
            {
                pbuf += 5;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_ADD_COUNTER:
            {
                pbuf += 7;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_REMOVE_COUNTER:
            {
                pbuf += 7;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_ATTACK:
            {
                pbuf += 8;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_BATTLE:
            {
                pbuf += 26;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_ATTACK_DISABLED:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_DAMAGE_STEP_START:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                break;
            }
            case MSG_DAMAGE_STEP_END:
            {
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshMzone(0);
                RefreshMzone(1);
                break;
            }
            case MSG_MISSED_EFFECT:
            {
                player = pbuf[0];
                pbuf += 8;
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                break;
            }
            case MSG_TOSS_COIN:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_TOSS_DICE:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += count;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_ROCK_PAPER_SCISSORS:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_HAND_RES:
            {
                pbuf += 1;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_ANNOUNCE_RACE:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 5;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_ANNOUNCE_ATTRIB:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 5;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_ANNOUNCE_CARD:
            case MSG_ANNOUNCE_NUMBER:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                count = BufferIO::Read<uint8_t>(pbuf);
                pbuf += 4 * count;
                WaitforResponse(player);
                {
                    const int remapped = remap_field_player(last_response);
                    if (remapped != -1)
                        NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                }
                return 1;
            }
            case MSG_CARD_HINT:
            {
                pbuf += 9;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_PLAYER_HINT:
            {
                pbuf += 6;
#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                break;
            }
            case MSG_TAG_SWAP:
            {
                player = BufferIO::Read<uint8_t>(pbuf);
                /*int mcount = */ BufferIO::Read<uint8_t>(pbuf);
                int ecount = BufferIO::Read<uint8_t>(pbuf);
                /*int pcount = */ BufferIO::Read<uint8_t>(pbuf);
                int hcount = BufferIO::Read<uint8_t>(pbuf);
                pbufw = pbuf + 4;
                pbuf += hcount * 4 + ecount * 4 + 4;
                const int remapped = remap_field_player(player);
                if (remapped != -1)
                    NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, offset, pbuf - offset);
                for (int i = 0; i < hcount; ++i)
                {
                    if (!(pbufw[3] & 0x80))
                        BufferIO::Write<int32_t>(pbufw, 0);
                    else
                        pbufw += 4;
                }
                for (int i = 0; i < ecount; ++i)
                {
                    if (!(pbufw[3] & 0x80))
                        BufferIO::Write<int32_t>(pbufw, 0);
                    else
                        pbufw += 4;
                }

#ifdef YGOPRO_SERVER_MODE
                BcastObs(offset, pbuf - offset, true);
#endif
                RefreshExtra(player);
                RefreshMzone(0, 0x81fff, 0);
                RefreshMzone(1, 0x81fff, 0);
                RefreshSzone(0, 0x681fff, 0);
                RefreshSzone(1, 0x681fff, 0);
                RefreshHand(0, 0x781fff, 0);
                RefreshHand(1, 0x781fff, 0);
                break;
            }
            case MSG_MATCH_KILL:
            {
                pbuf += 4;
                break;
            }
            }
        }
        return 0;
    }
    void TagDuel::GetResponse(DuelPlayer *dp, unsigned char *pdata, unsigned int len)
    {
        unsigned char resb[SIZE_RETURN_VALUE]{};
        if (len > UINT8_MAX)
            len = UINT8_MAX;
        std::memcpy(resb, pdata, len);
        last_replay_response_size = last_replay.WriteResponse(resb, len);
#ifdef YGOPRO_SERVER_MODE
        if (dp->type <= 3 && !IsBattleField())
        {
            set_active_field(pduel, dp->type);
            active_field = dp->type;
        }
        else
        {
            for (int i = 0; i < 4; ++i)
            {
                if (battle_pairs[i / 2][i % 2] == dp->type)
                {
                    set_active_field(pduel, 4 + i / 2);
                    active_field = 4 + i / 2;
                    break;
                }
            }
        }
#endif
        fprintf(stderr, "[DEBUG] GetResponse ENTER: dp->type=%d active_field=%d len=%u\n", (int)dp->type, (int)active_field, len);
        fflush(stderr);
        set_responseb(pduel, resb);
        players[dp->type]->state = 0xff;
        if (host_info.time_limit)
        {
            int resp_type = dp->type < 2 ? 0 : 1;
            if (time_limit[resp_type] >= time_elapsed)
                time_limit[resp_type] -= time_elapsed;
            else
                time_limit[resp_type] = 0;
            time_elapsed = 0;
#ifdef YGOPRO_SERVER_MODE
            if (time_backed[resp_type] > 0 && time_limit[resp_type] < host_info.time_limit && NetServer::IsCanIncreaseTime(last_game_msg, pdata, len))
            {
                ++time_limit[resp_type];
                ++time_compensator[resp_type];
                --time_backed[resp_type];
            }
#endif
        }
        Process();
        last_replay_response_size = 0;
    }
    void TagDuel::EndDuel()
    {
        if (!pduel)
            return;
        last_replay.EndRecord();
        std::vector<unsigned char> replay_buffer;
        replay_buffer.reserve(sizeof last_replay.pheader + last_replay.comp_size);
        BufferIO::VectorWrite(replay_buffer, last_replay.pheader);
        BufferIO::VectorWriteBlock(replay_buffer, last_replay.comp_data, last_replay.comp_size);
        NetServer::SendBufferToPlayer(players[0], STOC_REPLAY, replay_buffer.data(), replay_buffer.size());
        NetServer::ReSendToPlayer(players[1]);
        NetServer::ReSendToPlayer(players[2]);
        NetServer::ReSendToPlayer(players[3]);
#ifdef YGOPRO_SERVER_MODE
        if (!(replay_mode & REPLAY_MODE_WATCHER_NO_SEND))
        {
            for (auto oit = observers.begin(); oit != observers.end(); ++oit)
                NetServer::ReSendToPlayer(*oit);
            NetServer::ReSendToPlayers(cache_recorder, replay_recorder);
        }
        else
        {
            NetServer::ReSendToPlayer(replay_recorder);
        }
#else
        for (auto oit = observers.begin(); oit != observers.end(); ++oit)
            NetServer::ReSendToPlayer(*oit);
#endif
        end_duel(pduel);
        event_del(etimer);
        pduel = 0;
    }
    void TagDuel::WaitforResponse(int playerid)
    {
        fprintf(stderr, "[DEBUG] WaitforResponse: playerid=%d active_field=%d\n", (int)playerid, (int)active_field);
        fflush(stderr);
        int remapped = 0;
        int oppRemapped = 0;
#ifdef YGOPRO_SERVER_MODE
        if (!IsBattleField() && playerid != 1)
        {
            remapped = active_field;
        }
        if (IsBattleField())
        {
            remapped = remap_field_player(playerid);
            oppRemapped = remap_field_player(1 - playerid);
        }
#endif
        last_response = playerid;
        unsigned char msg = MSG_WAITING;
#ifdef YGOPRO_SERVER_MODE
        if (IsBattleField())
        {
            NetServer::SendPacketToPlayer(players[oppRemapped], STOC_GAME_MSG, msg);
        }
#endif
        if (host_info.time_limit)
        {
            STOC_TimeLimit sctl;
            sctl.player = playerid;
            sctl.left_time = time_limit[playerid];
#ifdef YGOPRO_SERVER_MODE
            if (IsBattleField())
            {
                NetServer::SendPacketToPlayer(players[oppRemapped], STOC_TIME_LIMIT, sctl);
            }
#endif
            players[remapped]->state = CTOS_TIME_CONFIRM;
        }
        else
            players[remapped]->state = CTOS_RESPONSE;
        fprintf(stderr, "[DEBUG] WaitforResponseAfterRewrite: playerid=%d active_field=%d\n", (int)playerid, (int)active_field);
        fflush(stderr);
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::RequestField(DuelPlayer *dp)
    {
        if (dp->type > 3)
            return;
        uint8_t player = (dp->type > 1) ? 1 : 0;
        NetServer::SendPacketToPlayer(dp, STOC_DUEL_START);

        uint8_t buf[1024];
        uint8_t *temp_buf = buf;
        auto WriteMsg = [&](const std::function<void(uint8_t *&)> &writer)
        {
            temp_buf = buf;
            writer(temp_buf);
            NetServer::SendBufferToPlayer(dp, STOC_GAME_MSG, buf, temp_buf - buf);
        };

        WriteMsg([&](uint8_t *&pbuf)
                 {
		BufferIO::Write<uint8_t>(pbuf, MSG_START);
		BufferIO::Write<uint8_t>(pbuf, player);
		BufferIO::Write<uint8_t>(pbuf, host_info.duel_rule);
		BufferIO::Write<int32_t>(pbuf, host_info.start_lp);
		BufferIO::Write<int32_t>(pbuf, host_info.start_lp);
		BufferIO::Write<uint16_t>(pbuf, 0);
		BufferIO::Write<uint16_t>(pbuf, 0);
		BufferIO::Write<uint16_t>(pbuf, 0);
		BufferIO::Write<uint16_t>(pbuf, 0); });

        uint8_t newturn_count = turn_count % 4;
        if (newturn_count == 0)
            newturn_count = 4;
        for (uint8_t i = 0; i < newturn_count; ++i)
        {
            WriteMsg([&](uint8_t *&pbuf)
                     {
			BufferIO::Write<uint8_t>(pbuf, MSG_NEW_TURN);
			BufferIO::Write<uint8_t>(pbuf, i % 2); });
        }

        WriteMsg([&](uint8_t *&pbuf)
                 {
		BufferIO::Write<uint8_t>(pbuf, MSG_NEW_PHASE);
		BufferIO::Write<uint16_t>(pbuf, phase); });

        WriteMsg([&](uint8_t *&pbuf)
                 {
		auto length = query_field_info(pduel, 0, pbuf);
		pbuf += length; });

        RefreshMzone(1 - player, 0xefffff, 0, active_field, dp);
        RefreshMzone(player, 0xefffff, 0, active_field, dp);
        RefreshSzone(1 - player, 0xefffff, 0, active_field, dp);
        RefreshSzone(player, 0xefffff, 0, active_field, dp);
        RefreshHand(1 - player, 0xefffff, 0, active_field, dp);
        RefreshHand(player, 0xefffff, 0, active_field, dp);
        RefreshGrave(1 - player, 0xefffff, 0, active_field, dp);
        RefreshGrave(player, 0xefffff, 0, active_field, dp);
        RefreshExtra(1 - player, 0xefffff, 0, active_field, dp);
        RefreshExtra(player, 0xefffff, 0, active_field, dp);
        RefreshRemoved(1 - player, 0xefffff, 0, active_field, dp);
        RefreshRemoved(player, 0xefffff, 0, active_field, dp);

        uint8_t query_buffer[SIZE_QUERY_BUFFER];
        for (uint8_t i = 0; i < 2; ++i)
        {
            // get decktop card
            auto qlen = query_field_card(pduel, 0, i, LOCATION_DECK, QUERY_CODE | QUERY_POSITION, query_buffer, 0);
            if (!qlen)
                continue; // no cards in deck
            uint8_t *qbuf = query_buffer;
            uint32_t code = 0;
            uint32_t position = 0;
            while (qbuf < query_buffer + qlen)
            {
                auto clen = BufferIO::Read<int32_t>(qbuf);
                if (qbuf + clen - 4 == query_buffer + qlen)
                {
                    // last card
                    code = *(uint32_t *)(qbuf + 4);
                    position = GetPosition(qbuf, 8);
                }
                qbuf += clen - 4;
            }
            if (position & POS_FACEUP)
                code |= 0x80000000; // mark as reversed
            if (deck_reversed || position & POS_FACEUP)
                WriteMsg([&](uint8_t *&pbuf)
                         {
				BufferIO::Write<uint8_t>(pbuf, MSG_DECK_TOP);
				BufferIO::Write<uint8_t>(pbuf, i);
				BufferIO::Write<uint8_t>(pbuf, 0);
				BufferIO::Write<int32_t>(pbuf, code); });
        }

        /*
        if(dp == cur_player[last_response])
            WaitforResponse(last_response);
        */
        STOC_TimeLimit sctl;
        sctl.player = 1 - last_response;
        sctl.left_time = time_limit[1 - last_response];
        NetServer::SendPacketToPlayer(dp, STOC_TIME_LIMIT, sctl);
        sctl.player = last_response;
        sctl.left_time = time_limit[last_response] - time_elapsed;
        NetServer::SendPacketToPlayer(dp, STOC_TIME_LIMIT, sctl);

        NetServer::SendPacketToPlayer(dp, STOC_FIELD_FINISH);
    }
#endif // YGOPRO_SERVER_MODE
    void TagDuel::TimeConfirm(DuelPlayer *dp)
    {
        if (host_info.time_limit == 0)
            return;
        if (dp != players[last_response])
            return;
        players[last_response]->state = CTOS_RESPONSE;
#ifdef YGOPRO_SERVER_MODE
        int resp_type = dp->type < 2 ? 0 : 1;
        if (time_elapsed < 10 && time_elapsed <= time_compensator[resp_type])
        {
            time_compensator[resp_type] -= time_elapsed;
            time_elapsed = 0;
        }
        else
        {
            time_limit[resp_type] -= time_elapsed;
            time_elapsed = 0;
        }
#else
        if (time_elapsed < 10)
            time_elapsed = 0;
#endif // YGOPRO_SERVER_MODE
    }
    inline int TagDuel::WriteUpdateData(int player, int location, unsigned int flag, unsigned char *&qbuf, int use_cache, int field)
    {
        flag |= (QUERY_CODE | QUERY_POSITION);
        BufferIO::Write<uint8_t>(qbuf, MSG_UPDATE_DATA);
        BufferIO::Write<uint8_t>(qbuf, player);
        BufferIO::Write<uint8_t>(qbuf, location);
        int len = query_field_card(pduel, field, player, location, flag, qbuf, use_cache);
        return len;
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::RefreshMzone(int player, int flag, int use_cache, int field, DuelPlayer *dp)
#else
    void TagDuel::RefreshMzone(int player, int flag, int use_cache, int field)
#endif // YGOPRO_SERVER_MODE
    {
#ifdef YGOPRO_SERVER_MODE
        if (field == -1)
            field = active_field;
#endif
        std::array<unsigned char, SIZE_QUERY_BUFFER> query_buffer;
        auto qbuf = query_buffer.data();
        auto len = WriteUpdateData(player, LOCATION_MZONE, flag, qbuf, use_cache, field);
        std::vector<std::pair<unsigned char *, int>> hidden_segments;
        int qlen = 0;
        while (qlen < len)
        {
            int clen = BufferIO::Read<int32_t>(qbuf);
            qlen += clen;
            if (clen <= LEN_HEADER)
                continue;
            auto position = GetPosition(qbuf, 8);
            const bool hide_code = NetServer::ShouldHideFacedownCode(position);
            position = NetServer::StripRevealFlag(qbuf, 8);
            if (hide_code)
                hidden_segments.emplace_back(qbuf, clen);
            qbuf += clen - 4;
        }
        int remapped = remap_field_player(player);
#ifdef YGOPRO_SERVER_MODE
        if (!dp || dp == players[remapped] && remapped != -1)
#endif
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
#ifdef YGOPRO_SERVER_MODE
        if (!dp)
            NetServer::ReSendToPlayer(replay_recorder);
#endif
        for (const auto &segment : hidden_segments)
            std::memset(segment.first, 0, segment.second - 4);
        remapped = remap_field_player(1 - player);
#ifdef YGOPRO_SERVER_MODE
        if (!dp || dp == players[remapped] && remapped != -1)
#endif
        {
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::ReSendToPlayer(*pit);
#ifdef YGOPRO_SERVER_MODE
        }
        if (!dp)
            NetServer::ReSendToPlayer(cache_recorder);
#endif
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::RefreshSzone(int player, int flag, int use_cache, int field, DuelPlayer *dp)
#else
        void TagDuel::RefreshSzone(int player, int flag, int use_cache, int field)
#endif // YGOPRO_SERVER_MODE
    {
#ifdef YGOPRO_SERVER_MODE
        if (field == -1)
            field = active_field;
#endif
        std::array<unsigned char, SIZE_QUERY_BUFFER> query_buffer;
        auto qbuf = query_buffer.data();
        auto len = WriteUpdateData(player, LOCATION_SZONE, flag, qbuf, use_cache, field);
        std::vector<std::pair<unsigned char *, int>> hidden_segments;
        int qlen = 0;
        while (qlen < len)
        {
            int clen = BufferIO::Read<int32_t>(qbuf);
            qlen += clen;
            if (clen <= LEN_HEADER)
                continue;
            auto position = GetPosition(qbuf, 8);
            const bool hide_code = NetServer::ShouldHideFacedownCode(position);
            position = NetServer::StripRevealFlag(qbuf, 8);
            if (hide_code)
                hidden_segments.emplace_back(qbuf, clen);
            qbuf += clen - 4;
        }
        int remapped = remap_field_player(player);
#ifdef YGOPRO_SERVER_MODE
        if (!dp || dp == players[remapped] && remapped != -1)
#endif
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
#ifdef YGOPRO_SERVER_MODE
        if (!dp)
            NetServer::ReSendToPlayer(replay_recorder);
#endif
        for (const auto &segment : hidden_segments)
            std::memset(segment.first, 0, segment.second - 4);
        remapped = remap_field_player(1 - player);
#ifdef YGOPRO_SERVER_MODE
        if (!dp || dp == players[remapped] && remapped != -1)
#endif
        {
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::ReSendToPlayer(*pit);
#ifdef YGOPRO_SERVER_MODE
        }
        if (!dp)
            NetServer::ReSendToPlayer(cache_recorder);
#endif
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::RefreshHand(int player, int flag, int use_cache, int field, DuelPlayer *dp)
#else
            void TagDuel::RefreshHand(int player, int flag, int use_cache, int field)
#endif // YGOPRO_SERVER_MODE
    {
#ifdef YGOPRO_SERVER_MODE
        if (field == -1)
            field = active_field;
#endif
        std::array<unsigned char, SIZE_QUERY_BUFFER> query_buffer;
        auto qbuf = query_buffer.data();
        auto len = WriteUpdateData(player, LOCATION_HAND, flag, qbuf, use_cache, field);
        int remapped = remap_field_player(player);
#ifdef YGOPRO_SERVER_MODE
        if (!dp || dp == players[remapped] && remapped != -1)
#endif
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
#ifdef YGOPRO_SERVER_MODE
        if (!dp)
            NetServer::ReSendToPlayer(replay_recorder);
#endif
        int qlen = 0;
        while (qlen < len)
        {
            int slen = BufferIO::Read<int32_t>(qbuf);
            qlen += slen;
            if (slen <= LEN_HEADER)
                continue;
            auto position = GetPosition(qbuf, 8);
            if (!(position & POS_FACEUP))
                std::memset(qbuf, 0, slen - 4);
            qbuf += slen - 4;
        }
        remapped = remap_field_player(1 - player);
#ifdef YGOPRO_SERVER_MODE
        if (!dp || dp == players[remapped] && remapped != -1)
#endif
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
#ifdef YGOPRO_SERVER_MODE
        if (!dp && remapped != -1)
#endif
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::ReSendToPlayer(*pit);
#ifdef YGOPRO_SERVER_MODE
        if (!dp)
            NetServer::ReSendToPlayer(cache_recorder);
#endif
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::RefreshGrave(int player, int flag, int use_cache, int field, DuelPlayer *dp)
#else
            void TagDuel::RefreshGrave(int player, int flag, int use_cache, int field)
#endif // YGOPRO_SERVER_MODE
    {
#ifdef YGOPRO_SERVER_MODE
        if (field == -1)
            field = active_field;
#endif
        std::array<unsigned char, SIZE_QUERY_BUFFER> query_buffer;
        auto qbuf = query_buffer.data();
        auto len = WriteUpdateData(player, LOCATION_GRAVE, flag, qbuf, use_cache, field);
#ifdef YGOPRO_SERVER_MODE
        for (int i = 0; i < 2; ++i)
        {
            int remapped = remap_field_player(i);
            if (!dp || dp == players[i] && remapped != -1)
                NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
        }
        if (!dp)
#else
                NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);
                NetServer::ReSendToPlayer(players[1]);
                NetServer::ReSendToPlayer(players[2]);
                NetServer::ReSendToPlayer(players[3]);
#endif
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::ReSendToPlayer(*pit);
#ifdef YGOPRO_SERVER_MODE
        if (!dp)
            NetServer::ReSendToPlayers(cache_recorder, replay_recorder);
#endif
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::RefreshExtra(int player, int flag, int use_cache, int field, DuelPlayer *dp)
#else
            void TagDuel::RefreshExtra(int player, int flag, int use_cache, int field)
#endif // YGOPRO_SERVER_MODE
    {
#ifdef YGOPRO_SERVER_MODE
        if (field == -1)
            field = active_field;
#endif
        std::array<unsigned char, SIZE_QUERY_BUFFER> query_buffer;
        auto qbuf = query_buffer.data();
        auto len = WriteUpdateData(player, LOCATION_EXTRA, flag, qbuf, use_cache, field);
        int remapped = remap_field_player(player);
#ifdef YGOPRO_SERVER_MODE
        if (!dp || dp == players[remapped] && remapped != -1)
#endif
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
#ifdef YGOPRO_SERVER_MODE
        if (!dp)
            NetServer::ReSendToPlayer(replay_recorder);
        int qlen = 0;
        while (qlen < len)
        {
            int clen = BufferIO::Read<int32_t>(qbuf);
            qlen += clen;
            if (clen <= LEN_HEADER)
                continue;
            auto position = GetPosition(qbuf, 8);
            if (position & POS_FACEDOWN)
                memset(qbuf, 0, clen - 4);
            qbuf += clen - 4;
        }
        remapped = remap_field_player(1 - player);
        if (!dp || dp == players[remapped] && remapped != -1)
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
        if (!dp)
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::ReSendToPlayer(*pit);
        if (!dp)
            NetServer::ReSendToPlayer(cache_recorder);
#endif
    }
#ifdef YGOPRO_SERVER_MODE
    void TagDuel::RefreshRemoved(int player, int flag, int use_cache, int field, DuelPlayer *dp)
    {
        if (field == -1)
            field = active_field;
        std::array<unsigned char, SIZE_QUERY_BUFFER> query_buffer;
        auto qbuf = query_buffer.data();
        auto len = WriteUpdateData(player, LOCATION_REMOVED, flag, qbuf, use_cache, field);
        int remapped = remap_field_player(player);
        if (!dp || dp == players[remapped] && remapped != -1)
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
        if (!dp)
            NetServer::ReSendToPlayer(replay_recorder);
        int qlen = 0;
        while (qlen < len)
        {
            int clen = BufferIO::Read<int32_t>(qbuf);
            qlen += clen;
            if (clen <= LEN_HEADER)
                continue;
            auto position = GetPosition(qbuf, 8);
            if (position & POS_FACEDOWN)
                memset(qbuf, 0, clen - 4);
            qbuf += clen - 4;
        }
        remapped = remap_field_player(1 - player);
        if (!dp || dp == players[remapped] && remapped != -1)
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer.data(), len + 3);
        if (!dp)
            for (auto pit = observers.begin(); pit != observers.end(); ++pit)
                NetServer::ReSendToPlayer(*pit);
        if (!dp)
            NetServer::ReSendToPlayer(cache_recorder);
    }
#endif
    void TagDuel::RefreshSingle(int player, int location, int sequence, int flag, int field)
    {
#ifdef YGOPRO_SERVER_MODE
        if (field == -1)
            field = active_field;
#else
                if (field == -1)
                    field = 0;
#endif
        flag |= (QUERY_CODE | QUERY_POSITION);
        unsigned char query_buffer[0x1000];
        auto qbuf = query_buffer;
        BufferIO::Write<uint8_t>(qbuf, MSG_UPDATE_CARD);
        BufferIO::Write<uint8_t>(qbuf, player);
        BufferIO::Write<uint8_t>(qbuf, location);
        BufferIO::Write<uint8_t>(qbuf, sequence);
        int len = query_card(pduel, field, player, location, sequence, flag, qbuf, 0);
        int remapped = remap_field_player(player);
        if (len <= LEN_HEADER && remapped != -1)
        {
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer, len + 4);
#ifdef YGOPRO_SERVER_MODE
            NetServer::ReSendToPlayer(replay_recorder);
#endif
            return;
        }
        auto position = GetPosition(qbuf, 12);
        bool hide_code = position & POS_FACEDOWN;
        if (location & LOCATION_ONFIELD)
        {
            hide_code = NetServer::ShouldHideFacedownCode(position);
            position = NetServer::StripRevealFlag(qbuf, 12);
        }
        if (remapped != -1)
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer, len + 4);
#ifdef YGOPRO_SERVER_MODE
        NetServer::ReSendToPlayer(replay_recorder);
#endif
        if (hide_code)
        {
            BufferIO::Write<int32_t>(qbuf, 16);
            BufferIO::Write<uint32_t>(qbuf, QUERY_CODE | QUERY_POSITION);
            BufferIO::Write<uint32_t>(qbuf, 0);
            // keep the 4 bytes position data
            len = 16;
        }
        remapped = remap_field_player(1 - player);
        if (remapped != -1)
            NetServer::SendBufferToPlayer(players[remapped], STOC_GAME_MSG, query_buffer, len + 4);
        for (auto pit = observers.begin(); pit != observers.end(); ++pit)
            NetServer::ReSendToPlayer(*pit);
#ifdef YGOPRO_SERVER_MODE
        NetServer::ReSendToPlayer(cache_recorder);
#endif
    }
    uint32_t TagDuel::MessageHandler(intptr_t fduel, uint32_t type)
    {
        char msgbuf[1024];
        get_log_message(fduel, msgbuf);
        mainGame->AddDebugMsg(msgbuf);
        return 0;
    }
    void TagDuel::TagTimer(evutil_socket_t fd, short events, void *arg)
    {
        TagDuel *sd = static_cast<TagDuel *>(arg);
        sd->time_elapsed++;
        if (sd->time_elapsed >= sd->time_limit[sd->last_response] || sd->time_limit[sd->last_response] <= 0)
        {
            unsigned char wbuf[3];
            uint32_t player = sd->last_response;
            wbuf[0] = MSG_WIN;
            wbuf[1] = 1 - player;
            wbuf[2] = 0x3;
            NetServer::SendBufferToPlayer(sd->players[0], STOC_GAME_MSG, wbuf, 3);
            NetServer::ReSendToPlayer(sd->players[1]);
            NetServer::ReSendToPlayer(sd->players[2]);
            NetServer::ReSendToPlayer(sd->players[3]);
            sd->EndDuel();
            sd->DuelEndProc();
            event_del(sd->etimer);
            return;
        }
        timeval timeout = {1, 0};
        event_add(sd->etimer, &timeout);
    }
}
