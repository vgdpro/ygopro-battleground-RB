#include "data_manager.h"
#include "game.h"
#include "myfilesystem.h"
#include "../ocgcore/ocgapi.h"
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <random>
#include <sstream>
#include <sys/stat.h>
#include <unordered_set>
#if !defined(YGOPRO_SERVER_MODE) || defined(SERVER_ZIP_SUPPORT)
#include "client_card.h"
#endif

namespace ygo
{

    namespace
    {
        unsigned char scriptBuffer[0x100000]{};

        constexpr uint32_t CAPABILITY_CACHE_VERSION = 1;
        const char *CAPABILITY_CACHE_PATH = "./cache/card-capabilities.cache";
        const char *CARD_EXCLUSION_PATH = "./Iflist.txt";

        struct ScriptMetadata
        {
            std::string path;
            uint64_t size{};
            int64_t mtime{};
        };

        struct CapabilityCacheRecord
        {
            ScriptMetadata script;
            uint64_t capability_mask{};
        };

        bool get_script_metadata(const char *path, ScriptMetadata &metadata)
        {
#ifdef _WIN32
            struct _stat64 status{};
            if (_stat64(path, &status) != 0 || (status.st_mode & _S_IFDIR))
                return false;
#else
            struct stat status{};
            if (stat(path, &status) != 0 || S_ISDIR(status.st_mode))
                return false;
#endif
            metadata.path = path;
            metadata.size = (uint64_t)status.st_size;
            metadata.mtime = (int64_t)status.st_mtime;
            return true;
        }

        bool resolve_card_script(uint32_t code, ScriptMetadata &metadata)
        {
            char path[128]{};
#ifdef YGOPRO_SERVER_MODE
            mysnprintf(path, "./specials/c%u.lua", code);
            if (get_script_metadata(path, metadata))
                return true;
#endif
            mysnprintf(path, "./expansions/script/c%u.lua", code);
            if (get_script_metadata(path, metadata))
                return true;
            mysnprintf(path, "./script/c%u.lua", code);
            return get_script_metadata(path, metadata);
        }

        bool read_text_file(const std::string &path, std::string &content)
        {
            std::ifstream input(path, std::ios::binary);
            if (!input)
                return false;
            content.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
            return input.good() || input.eof();
        }

        bool parse_uint64(const std::string &field, uint64_t &value)
        {
            if (field.empty() || !std::all_of(field.begin(), field.end(), [](char character)
                                              { return character >= '0' && character <= '9'; }))
                return false;
            errno = 0;
            char *end{};
            unsigned long long parsed = std::strtoull(field.c_str(), &end, 10);
            if (errno == ERANGE || end == field.c_str() || *end)
                return false;
            value = static_cast<uint64_t>(parsed);
            return true;
        }

        bool parse_int64(const std::string &field, int64_t &value)
        {
            size_t digits_start = !field.empty() && field[0] == '-' ? 1 : 0;
            if (digits_start == field.size() ||
                !std::all_of(field.begin() + digits_start, field.end(), [](char character)
                             { return character >= '0' && character <= '9'; }))
                return false;
            errno = 0;
            char *end{};
            long long parsed = std::strtoll(field.c_str(), &end, 10);
            if (errno == ERANGE || end == field.c_str() || *end)
                return false;
            value = static_cast<int64_t>(parsed);
            return true;
        }

        uint32_t get_random_index(std::mt19937 &random, uint32_t upper)
        {
            if (upper == std::numeric_limits<uint32_t>::max())
                return random();
            uint32_t range = upper + 1;
            uint32_t bound = -range % range;
            uint32_t value = random();
            while (value < bound)
                value = random();
            return value % range;
        }

        size_t lua_long_bracket_end(const std::string &source, size_t start)
        {
            if (start >= source.size() || source[start] != '[')
                return std::string::npos;
            size_t equals = start + 1;
            while (equals < source.size() && source[equals] == '=')
                ++equals;
            if (equals >= source.size() || source[equals] != '[')
                return std::string::npos;
            for (size_t index = equals + 1; index < source.size(); ++index)
            {
                if (source[index] != ']')
                    continue;
                size_t close = index + 1;
                size_t count = equals - start - 1;
                while (count && close < source.size() && source[close] == '=')
                {
                    --count;
                    ++close;
                }
                if (!count && close < source.size() && source[close] == ']')
                    return close + 1;
            }
            return source.size();
        }

        std::string lua_code_only(const std::string &source)
        {
            std::string result(source.size(), ' ');
            for (size_t index = 0; index < source.size();)
            {
                if (source[index] == '\n' || source[index] == '\r')
                {
                    result[index] = source[index];
                    ++index;
                    continue;
                }
                if (source[index] == '-' && index + 1 < source.size() && source[index + 1] == '-')
                {
                    size_t long_start = index + 2;
                    size_t long_end = lua_long_bracket_end(source, long_start);
                    if (long_end != std::string::npos)
                    {
                        index = long_end;
                        continue;
                    }
                    while (index < source.size() && source[index] != '\n' && source[index] != '\r')
                        ++index;
                    continue;
                }
                if (source[index] == '\'' || source[index] == '"')
                {
                    char quote = source[index++];
                    while (index < source.size())
                    {
                        if (source[index] == '\\' && index + 1 < source.size())
                        {
                            index += 2;
                            continue;
                        }
                        if (source[index++] == quote)
                            break;
                    }
                    continue;
                }
                size_t long_end = lua_long_bracket_end(source, index);
                if (long_end != std::string::npos)
                {
                    index = long_end;
                    continue;
                }
                result[index] = source[index];
                ++index;
            }
            return result;
        }

        bool is_lua_identifier_char(char character)
        {
            return std::isalnum((unsigned char)character) || character == '_';
        }

        bool is_lua_token_at(const std::string &source, size_t start, size_t length)
        {
            return (start == 0 || !is_lua_identifier_char(source[start - 1])) &&
                   (start + length == source.size() || !is_lua_identifier_char(source[start + length]));
        }

        bool has_lua_call(const std::string &source, const char *name)
        {
            size_t name_length = std::strlen(name);
            for (size_t found = source.find(name); found != std::string::npos; found = source.find(name, found + name_length))
            {
                size_t end = found + name_length;
                if (!is_lua_token_at(source, found, name_length))
                    continue;
                while (end < source.size() && std::isspace((unsigned char)source[end]))
                    ++end;
                if (end < source.size() && source[end] == '(')
                    return true;
            }
            return false;
        }

        bool has_lua_token(const std::string &source, const char *name)
        {
            size_t name_length = std::strlen(name);
            for (size_t found = source.find(name); found != std::string::npos; found = source.find(name, found + name_length))
            {
                if (is_lua_token_at(source, found, name_length))
                    return true;
            }
            return false;
        }

        void detect_card_capabilities(const std::string &script, uint64_t &capability_mask)
        {
            std::string code = lua_code_only(script);
            capability_mask = 0;
            if (has_lua_call(code, "Duel.SpecialSummon") || has_lua_call(code, "Duel.SpecialSummonStep"))
                capability_mask |= CARD_CAPABILITY_SPECIAL_SUMMON;
        }

        bool load_capability_cache(std::unordered_map<uint32_t, CapabilityCacheRecord> &records)
        {
            std::ifstream input(CAPABILITY_CACHE_PATH);
            std::string header;
            if (!std::getline(input, header) || header != "card-capabilities\t1")
                return false;
            std::string line;
            while (std::getline(input, line))
            {
                std::istringstream parser(line);
                std::string field;
                CapabilityCacheRecord record;
                if (!std::getline(parser, field, '\t'))
                    return false;
                uint64_t parsed_code{};
                if (!parse_uint64(field, parsed_code) || !parsed_code || parsed_code > std::numeric_limits<uint32_t>::max() ||
                    !std::getline(parser, record.script.path, '\t') || record.script.path.empty() ||
                    !std::getline(parser, field, '\t') || !parse_uint64(field, record.script.size))
                    return false;
                if (!std::getline(parser, field, '\t') || !parse_int64(field, record.script.mtime))
                    return false;
                if (!std::getline(parser, field, '\t') || !parse_uint64(field, record.capability_mask))
                    return false;
                records[static_cast<uint32_t>(parsed_code)] = record;
            }
            return true;
        }

        void save_capability_cache(const std::unordered_map<uint32_t, CapabilityCacheRecord> &records)
        {
            if (!FileSystem::IsDirExists("./cache") && !FileSystem::MakeDir("./cache"))
                return;
            std::vector<uint32_t> codes;
            codes.reserve(records.size());
            for (const auto &entry : records)
                codes.push_back(entry.first);
            std::sort(codes.begin(), codes.end());
            std::ofstream output(CAPABILITY_CACHE_PATH, std::ios::trunc);
            if (!output)
                return;
            output << "card-capabilities\t" << CAPABILITY_CACHE_VERSION << '\n';
            for (uint32_t code : codes)
            {
                const auto &record = records.at(code);
                output << code << '\t' << record.script.path << '\t' << record.script.size << '\t'
                       << record.script.mtime << '\t' << record.capability_mask << '\n';
            }
        }

        std::unordered_set<uint32_t> load_card_exclusions()
        {
            std::unordered_set<uint32_t> exclusions;
            std::ifstream input(CARD_EXCLUSION_PATH);
            std::string line;
            while (std::getline(input, line))
            {
                const char *start = line.c_str();
                while (*start && std::isspace((unsigned char)*start))
                    ++start;
                if (!*start || *start == '#')
                    continue;
                char *end{};
                uint32_t code = (uint32_t)std::strtoul(start, &end, 10);
                if (code && end != start)
                    exclusions.insert(code);
            }
            return exclusions;
        }
    }

    DataManager dataManager;
#ifdef YGOPRO_SERVER_MODE
    static const char SELECT_STMT[] = "SELECT datas.id, datas.ot, datas.alias, datas.setcode, datas.type, datas.atk, datas.def, datas.level, datas.race, datas.attribute, datas.category FROM datas";
#else
    static const char SELECT_STMT[] = "SELECT datas.id, datas.ot, datas.alias, datas.setcode, datas.type, datas.atk, datas.def, datas.level, datas.race, datas.attribute, datas.category,"
                                      " texts.name, texts.desc, texts.str1, texts.str2, texts.str3, texts.str4, texts.str5, texts.str6, texts.str7, texts.str8,"
                                      " texts.str9, texts.str10, texts.str11, texts.str12, texts.str13, texts.str14, texts.str15, texts.str16 FROM datas INNER JOIN texts ON datas.id = texts.id";
#endif
    static constexpr int DATAS_COUNT = 11;

    static constexpr int CARD_ARTWORK_VERSIONS_OFFSET = 20;
    static inline bool is_alternative(uint32_t code, uint32_t alias)
    {
        return alias && (alias < code + CARD_ARTWORK_VERSIONS_OFFSET) && (code < alias + CARD_ARTWORK_VERSIONS_OFFSET);
    }

    DataManager::DataManager() : _datas(32768), _strings(32768)
    {
        extra_setcode = {
            {8512558u, {0x8f, 0x54, 0x59, 0x82, 0x13a}},
            {55088578u, {0x8f, 0x54, 0x59, 0x82, 0x13a}},
        };
    }
    bool DataManager::ReadDB(sqlite3 *pDB)
    {
        sqlite3_stmt *pStmt = nullptr;
        int texts_offset = DATAS_COUNT;
        if (sqlite3_prepare_v2(pDB, SELECT_STMT, -1, &pStmt, nullptr) != SQLITE_OK)
            return Error(pDB, pStmt);
#ifndef YGOPRO_SERVER_MODE
        wchar_t strBuffer[4096];
#endif
        for (int step = sqlite3_step(pStmt); step != SQLITE_DONE; step = sqlite3_step(pStmt))
        {
            if (step != SQLITE_ROW)
                return Error(pDB, pStmt);
            uint32_t code = static_cast<uint32_t>(sqlite3_column_int64(pStmt, 0));
            auto &cd = _datas[code];
            cd.code = code;
            cd.ot = sqlite3_column_int(pStmt, 1);
            cd.alias = sqlite3_column_int(pStmt, 2);
            uint64_t setcode = static_cast<uint64_t>(sqlite3_column_int64(pStmt, 3));
            write_setcode(cd.setcode, setcode);
            cd.type = static_cast<decltype(cd.type)>(sqlite3_column_int64(pStmt, 4));
            cd.attack = sqlite3_column_int(pStmt, 5);
            cd.defense = sqlite3_column_int(pStmt, 6);
            if (cd.type & TYPE_LINK)
            {
                cd.link_marker = cd.defense;
                cd.defense = 0;
            }
            else
                cd.link_marker = 0;
            uint32_t level = static_cast<uint32_t>(sqlite3_column_int64(pStmt, 7));
            cd.level = level & 0xff;
            cd.lscale = (level >> 24) & 0xff;
            cd.rscale = (level >> 16) & 0xff;
            cd.race = static_cast<decltype(cd.race)>(sqlite3_column_int64(pStmt, 8));
            cd.attribute = static_cast<decltype(cd.attribute)>(sqlite3_column_int64(pStmt, 9));
            cd.category = static_cast<decltype(cd.category)>(sqlite3_column_int64(pStmt, 10));
            // rule_code
            if (cd.code == 5405695)
            {
                cd.rule_code = cd.alias;
                cd.alias = 0;
            }
            else if (cd.alias && !(cd.type & TYPE_TOKEN) && !is_alternative(cd.code, cd.alias))
            {
                cd.rule_code = cd.alias;
                cd.alias = 0;
            }
#ifndef YGOPRO_SERVER_MODE
            auto &cs = _strings[code];
            if (const char *text = (const char *)sqlite3_column_text(pStmt, texts_offset + 0))
            {
                BufferIO::DecodeUTF8(text, strBuffer);
                cs.name = strBuffer;
            }
            if (const char *text = (const char *)sqlite3_column_text(pStmt, texts_offset + 1))
            {
                BufferIO::DecodeUTF8(text, strBuffer);
                cs.text = strBuffer;
            }
            for (int i = 0; i < DESC_COUNT; ++i)
            {
                if (const char *text = (const char *)sqlite3_column_text(pStmt, (texts_offset + 2) + i))
                {
                    BufferIO::DecodeUTF8(text, strBuffer);
                    cs.desc[i] = strBuffer;
                }
            }
#endif // YGOPRO_SERVER_MODE
        }
        sqlite3_finalize(pStmt);
        for (auto &entry : _datas)
        {
            auto &cd = entry.second;
            if (cd.rule_code || !cd.alias || (cd.type & TYPE_TOKEN))
                continue;
            auto it = _datas.find(cd.alias);
            if (it == _datas.end())
                continue;
            cd.rule_code = it->second.rule_code;
        }
        for (const auto &entry : extra_setcode)
        {
            const auto &code = entry.first;
            const auto &list = entry.second;
            if (list.size() > SIZE_SETCODE || list.empty())
                continue;
            auto it = _datas.find(code);
            if (it == _datas.end())
                continue;
            std::memcpy(it->second.setcode, list.data(), list.size() * sizeof(uint16_t));
        }
        return true;
    }
    bool DataManager::LoadDB(const char *file)
    {
#if defined(YGOPRO_SERVER_MODE) && !defined(SERVER_ZIP_SUPPORT)
        bool ret{};
        sqlite3 *pDB{};
        if (sqlite3_open_v2(file, &pDB, SQLITE_OPEN_READONLY, 0) != SQLITE_OK)
            ret = Error(pDB);
        else
            ret = ReadDB(pDB);
        sqlite3_close(pDB);
#else
        auto reader = IrrFileSystem->createAndOpenFile(file);
        if (reader == nullptr)
        {
            mysnprintf(errmsg, "File does not exist or failed to unzip: %s", file);
            return false;
        }

        sqlite3 *db_handle = nullptr;
        if (sqlite3_open(":memory:", &db_handle) != SQLITE_OK)
        {
            Error(db_handle, nullptr);
            sqlite3_close(db_handle);
            reader->drop();
            return false;
        }

        sqlite3_int64 sz = reader->getSize();
        unsigned char *buffer = (unsigned char *)sqlite3_malloc64(sz);
        if (!buffer)
        {
            Error(db_handle, nullptr);
            sqlite3_close(db_handle);
            reader->drop();
            return false;
        }

        reader->read(buffer, sz);
        reader->drop();
        // force rollback-journal mode by setting header bytes 18 and 19 to 0x01
        if (sz >= 20 && buffer[18] == 0x02)
        {
            buffer[18] = 0x01;
            buffer[19] = 0x01;
        }
        int rc = sqlite3_deserialize(
            db_handle,
            nullptr,
            buffer,
            sz,
            sz,
            SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY);
        if (rc != SQLITE_OK)
        {
            Error(db_handle, nullptr);
            sqlite3_close(db_handle);
            return false;
        }
        bool ret = ReadDB(db_handle);
        sqlite3_close(db_handle);
#endif // YGOPRO_SERVER_MODE
        if (ret)
            BuildRandomCardPools();
        return ret;
    }
    bool DataManager::LoadStrings(const char *file)
    {
        FILE *fp = myfopen(file, "r");
        if (!fp)
            return false;
        char linebuf[TEXT_LINE_SIZE]{};
        while (std::fgets(linebuf, sizeof linebuf, fp))
        {
            ReadStringConfLine(linebuf);
        }
        std::fclose(fp);
        return true;
    }
#ifndef YGOPRO_SERVER_MODE
    bool DataManager::LoadStrings(irr::io::IReadFile *reader)
    {
        char ch{};
        std::string linebuf;
        while (reader->read(&ch, 1))
        {
            if (ch == '\0')
                break;
            linebuf.push_back(ch);
            if (ch == '\n' || linebuf.size() >= TEXT_LINE_SIZE - 1)
            {
                ReadStringConfLine(linebuf.data());
                linebuf.clear();
            }
        }
        if (!linebuf.empty())
        {
            ReadStringConfLine(linebuf.data());
        }
        reader->drop();
        return true;
    }
#endif
    void DataManager::ReadStringConfLine(const char *linebuf)
    {
        if (linebuf[0] != '!')
            return;
        char strbuf[TEXT_LINE_SIZE]{};
        uint32_t value{};
        wchar_t strBuffer[4096]{};
        if (std::sscanf(linebuf, "!%63s", strbuf) != 1)
            return;
        if (!std::strcmp(strbuf, "system"))
        {
            if (std::sscanf(&linebuf[7], "%u %240[^\n]", &value, strbuf) != 2)
                return;
            BufferIO::DecodeUTF8(strbuf, strBuffer);
            _sysStrings.emplace(value, strBuffer);
#ifndef YGOPRO_SERVER_MODE
        }
        else if (!std::strcmp(strbuf, "victory"))
        {
            if (std::sscanf(&linebuf[8], "%x %240[^\n]", &value, strbuf) != 2)
                return;
            BufferIO::DecodeUTF8(strbuf, strBuffer);
            _victoryStrings.emplace(value, strBuffer);
        }
        else if (!std::strcmp(strbuf, "counter"))
        {
            if (std::sscanf(&linebuf[8], "%x %240[^\n]", &value, strbuf) != 2)
                return;
            BufferIO::DecodeUTF8(strbuf, strBuffer);
            _counterStrings[value] = strBuffer;
        }
        else if (!std::strcmp(strbuf, "setname"))
        {
            // using tab for comment
            if (std::sscanf(&linebuf[8], "%x %240[^\t\n]", &value, strbuf) != 2)
                return;
            BufferIO::DecodeUTF8(strbuf, strBuffer);
            _setnameStrings[value] = strBuffer;
#endif
        }
    }
    bool DataManager::Error(sqlite3 *pDB, sqlite3_stmt *pStmt)
    {
        if (const char *msg = sqlite3_errmsg(pDB))
            mysnprintf(errmsg, "sqlite3_errmsg: %s", msg);
        else
            errmsg[0] = '\0';
        sqlite3_finalize(pStmt);
        return false;
    }
    code_pointer DataManager::GetCodePointer(uint32_t code) const
    {
        return _datas.find(code);
    }
#ifndef YGOPRO_SERVER_MODE
    string_pointer DataManager::GetStringPointer(uint32_t code) const
    {
        return _strings.find(code);
    }
#endif // YGOPRO_SERVER_MODE
    bool DataManager::GetData(uint32_t code, CardData *pData) const
    {
        auto cdit = _datas.find(code);
        if (cdit == _datas.end())
            return false;
        if (pData)
        {
            std::memcpy(pData, &cdit->second, sizeof(CardData));
        }
        return true;
    }
#ifndef YGOPRO_SERVER_MODE
    bool DataManager::GetString(uint32_t code, CardString *pStr) const
    {
        auto csit = _strings.find(code);
        if (csit == _strings.end())
        {
            pStr->name = unknown_string;
            pStr->text = unknown_string;
            return false;
        }
        *pStr = csit->second;
        return true;
    }
    const wchar_t *DataManager::GetName(uint32_t code) const
    {
        auto csit = _strings.find(code);
        if (csit == _strings.end())
            return unknown_string;
        if (!csit->second.name.empty())
            return csit->second.name.c_str();
        return unknown_string;
    }
    const wchar_t *DataManager::GetText(uint32_t code) const
    {
        auto csit = _strings.find(code);
        if (csit == _strings.end())
            return unknown_string;
        if (!csit->second.text.empty())
            return csit->second.text.c_str();
        return unknown_string;
    }
    const wchar_t *DataManager::GetDesc(uint32_t strCode) const
    {
        if (strCode <= MAX_STRING_ID)
            return GetSysString(strCode);
        unsigned int code = (strCode >> 4) & 0x0fffffff;
        unsigned int offset = strCode & 0xf;
        auto csit = _strings.find(code);
        if (csit == _strings.end())
            return unknown_string;
        if (!csit->second.desc[offset].empty())
            return csit->second.desc[offset].c_str();
        return unknown_string;
    }
#endif // YGOPRO_SERVER_MODE
    const wchar_t *DataManager::GetMapString(const wstring_map &table, uint32_t code) const
    {
        auto csit = table.find(code);
        if (csit == table.end())
            return unknown_string;
        return csit->second.c_str();
    }
    const wchar_t *DataManager::GetSysString(uint32_t code) const
    {
        return GetMapString(_sysStrings, code);
    }
#ifndef YGOPRO_SERVER_MODE
    const wchar_t *DataManager::GetVictoryString(uint32_t code) const
    {
        return GetMapString(_victoryStrings, code);
    }
    const wchar_t *DataManager::GetCounterName(uint32_t code) const
    {
        return GetMapString(_counterStrings, code);
    }
    const wchar_t *DataManager::GetSetName(uint32_t code) const
    {
        return GetMapString(_setnameStrings, code);
    }
    std::vector<uint32_t> DataManager::GetSetCodes(std::wstring setname) const
    {
        std::vector<uint32_t> matchingCodes;
        for (auto csit = _setnameStrings.begin(); csit != _setnameStrings.end(); ++csit)
        {
            const std::wstring &setnameString = csit->second;
            size_t start = 0;
            while (start < setnameString.size())
            { // handle "setname|another setname"
                auto pos = setnameString.find(L'|', start);
                std::wstring token;
                if (pos == std::wstring::npos)
                    token = setnameString.substr(start);
                else
                    token = setnameString.substr(start, pos - start);
                if (setname.size() < 2)
                {
                    // exact match for short set names to avoid too many results
                    if (token == setname)
                    {
                        matchingCodes.push_back(csit->first);
                        break;
                    }
                }
                else
                {
                    if (token.find(setname) != std::wstring::npos)
                    {
                        matchingCodes.push_back(csit->first);
                        break;
                    }
                }
                if (pos == std::wstring::npos)
                    break;
                start = pos + 1;
            }
        }
        return matchingCodes;
    }
    std::wstring DataManager::GetNumString(int num, bool bracket) const
    {
        if (!bracket)
            return std::to_wstring(num);
        std::wstring numBuffer{L"("};
        numBuffer.append(std::to_wstring(num));
        numBuffer.push_back(L')');
        return numBuffer;
    }
    const wchar_t *DataManager::FormatLocation(int location, int sequence) const
    {
        if (location == LOCATION_SZONE)
        {
            if (sequence < 5)
                return GetSysString(1003);
            else if (sequence == 5)
                return GetSysString(1008);
            else
                return GetSysString(1009);
        }
        int string_id = 0;
        for (int i = 0; i < 10; ++i)
        {
            if ((0x1U << i) == location)
            {
                string_id = STRING_ID_LOCATION + i;
                break;
            }
        }
        if (string_id)
            return GetSysString(string_id);
        else
            return unknown_string;
    }
    const wchar_t *DataManager::FormatLocation(ClientCard *card) const
    {
        if (!card)
            return unknown_string;
        return FormatLocation(card->location, card->sequence);
    }
    std::wstring DataManager::FormatAttribute(unsigned int attribute) const
    {
        std::wstring buffer;
        for (int i = 0; i < ATTRIBUTES_COUNT; ++i)
        {
            if (attribute & (0x1U << i))
            {
                if (!buffer.empty())
                    buffer.push_back(L'|');
                buffer.append(GetSysString(STRING_ID_ATTRIBUTE + i));
            }
        }
        if (buffer.empty())
            buffer = unknown_string;
        return buffer;
    }
    std::wstring DataManager::FormatRace(unsigned int race) const
    {
        std::wstring buffer;
        for (int i = 0; i < RACES_COUNT; ++i)
        {
            if (race & (0x1U << i))
            {
                if (!buffer.empty())
                    buffer.push_back(L'|');
                buffer.append(GetSysString(STRING_ID_RACE + i));
            }
        }
        if (buffer.empty())
            buffer = unknown_string;
        return buffer;
    }
    std::wstring DataManager::FormatType(unsigned int type) const
    {
        std::wstring buffer;
        for (int i = 0; i < TYPES_COUNT; ++i)
        {
            if (type & (0x1U << i))
            {
                if (!buffer.empty())
                    buffer.push_back(L'|');
                buffer.append(GetSysString(STRING_ID_TYPE + i));
            }
        }
        if (buffer.empty())
            buffer = unknown_string;
        return buffer;
    }
    std::wstring DataManager::FormatSetName(const uint16_t setcode[]) const
    {
        std::wstring buffer;
        for (int i = 0; i < SIZE_SETCODE; ++i)
        {
            if (!setcode[i])
                break;
            const wchar_t *setname = GetSetName(setcode[i]);
            if (!buffer.empty())
                buffer.push_back(L'|');
            buffer.append(setname);
        }
        if (buffer.empty())
            buffer = unknown_string;
        return buffer;
    }
    std::wstring DataManager::FormatLinkMarker(unsigned int link_marker) const
    {
        std::wstring buffer;
        if (link_marker & LINK_MARKER_TOP_LEFT)
            buffer.append(L"[\u2196]");
        if (link_marker & LINK_MARKER_TOP)
            buffer.append(L"[\u2191]");
        if (link_marker & LINK_MARKER_TOP_RIGHT)
            buffer.append(L"[\u2197]");
        if (link_marker & LINK_MARKER_LEFT)
            buffer.append(L"[\u2190]");
        if (link_marker & LINK_MARKER_RIGHT)
            buffer.append(L"[\u2192]");
        if (link_marker & LINK_MARKER_BOTTOM_LEFT)
            buffer.append(L"[\u2199]");
        if (link_marker & LINK_MARKER_BOTTOM)
            buffer.append(L"[\u2193]");
        if (link_marker & LINK_MARKER_BOTTOM_RIGHT)
            buffer.append(L"[\u2198]");
        return buffer;
    }
    wchar_t DataManager::NormalizeChar(wchar_t c)
    {
        // Convert Alphabet characters to uppercase to ignore case.
        if (c >= 0x0061 && c <= 0x007A)
        {
            return c - 0x0020;
        }
        // Normalize accented characters (Latin-1 Supplement).
        if ((c >= 0x00C0 && c <= 0x00C5) || (c >= 0x00E0 && c <= 0x00E5))
        {
            return L'A';
        }
        if (c == 0x00C7 || c == 0x00E7)
        {
            return L'C';
        }
        if ((c >= 0x00C8 && c <= 0x00CB) || (c >= 0x00E8 && c <= 0x00EB))
        {
            return L'E';
        }
        if ((c >= 0x00CC && c <= 0x00CF) || (c >= 0x00EC && c <= 0x00EF))
        {
            return L'I';
        }
        if (c == 0x00D1 || c == 0x00F1)
        {
            return L'N';
        }
        if ((c >= 0x00D2 && c <= 0x00D6) || (c >= 0x00F2 && c <= 0x00F6))
        {
            return L'O';
        }
        if ((c >= 0x00D9 && c <= 0x00DC) || (c >= 0x00F9 && c <= 0x00FC))
        {
            return L'U';
        }
        if (c == 0x00DD || c == 0x00FD || c == 0x00FF)
        {
            return L'Y';
        }
        return c;
    }
    void DataManager::NormalizeString(const wchar_t *src, wchar_t *dst, size_t dst_size)
    {
        size_t i = 0;
        for (; src[i] && i < dst_size - 1; ++i)
        {
            dst[i] = NormalizeChar(src[i]);
        }
        dst[i] = 0;
    }
    bool DataManager::CardNameContains(const wchar_t *haystack, const wchar_t *needle)
    {
        if (!needle[0])
        {
            return true;
        }
        if (!haystack)
        {
            return false;
        }
        wchar_t normalized_haystack[TEXT_LINE_SIZE]{};
        wchar_t normalized_needle[TEXT_LINE_SIZE]{};
        NormalizeString(haystack, normalized_haystack, TEXT_LINE_SIZE);
        NormalizeString(needle, normalized_needle, TEXT_LINE_SIZE);
        return std::wcsstr(normalized_haystack, normalized_needle) != nullptr;
    }
#endif // YGOPRO_SERVER_MODE
    uint32_t DataManager::CardReader(uint32_t code, card_data *pData)
    {
        if (!dataManager.GetData(code, pData))
            pData->clear();
        return 0;
    }
    uint32_t DataManager::RandomCardReader(uint64_t capability, uint32_t count, uint32_t random_seed, uint32_t *codes)
    {
        if (!count || !codes || (capability && (capability & (capability - 1))))
            return 0;
        const std::vector<uint32_t> *pool = &dataManager.random_card_pool;
        if (capability)
        {
            uint32_t index = 0;
            while ((capability >> index) != 1)
                ++index;
            pool = &dataManager.capability_pools[index];
        }
        uint32_t result_count = std::min<uint32_t>(count, (uint32_t)pool->size());
        if (!result_count)
            return 0;
        std::mt19937 random(random_seed);
        std::unordered_set<uint32_t> selected;
        selected.reserve(result_count);
        uint32_t output = 0;
        for (uint32_t upper = (uint32_t)pool->size() - result_count; upper < pool->size(); ++upper)
        {
            uint32_t candidate = get_random_index(random, upper);
            if (!selected.insert(candidate).second)
            {
                candidate = upper;
                selected.insert(candidate);
            }
            codes[output++] = (*pool)[candidate];
        }
        return output;
    }

    void DataManager::BuildRandomCardPools()
    {
        random_card_pool.clear();
        for (auto &pool : capability_pools)
            pool.clear();
        std::unordered_map<uint32_t, CapabilityCacheRecord> cached_records;
        if (!load_capability_cache(cached_records))
            cached_records.clear();
        std::unordered_map<uint32_t, CapabilityCacheRecord> current_records;
        std::unordered_set<uint32_t> exclusions = load_card_exclusions();
        std::vector<uint32_t> codes;
        codes.reserve(_datas.size());
        for (const auto &entry : _datas)
            codes.push_back(entry.first);
        std::sort(codes.begin(), codes.end());
        for (uint32_t code : codes)
        {
            auto &data = _datas[code];
            data.capability_mask = 0;
            if ((data.type & TYPE_TOKEN) || data.rule_code || is_alternative(data.code, data.alias) || exclusions.count(code))
                continue;
            ScriptMetadata metadata;
            if (resolve_card_script(code, metadata))
            {
                CapabilityCacheRecord record;
                auto cached = cached_records.find(code);
                if (cached != cached_records.end() && cached->second.script.path == metadata.path &&
                    cached->second.script.size == metadata.size && cached->second.script.mtime == metadata.mtime)
                {
                    record = cached->second;
                }
                else
                {
                    std::string script;
                    record.script = metadata;
                    if (!read_text_file(metadata.path, script))
                        continue;
                    detect_card_capabilities(script, record.capability_mask);
                }
                current_records[code] = record;
                data.capability_mask = record.capability_mask;
            }
            random_card_pool.push_back(code);
            for (uint32_t index = 0; index < capability_pools.size(); ++index)
            {
                if (data.capability_mask & (1ULL << index))
                    capability_pools[index].push_back(code);
            }
        }
        save_capability_cache(current_records);
    }
    unsigned char *DataManager::ScriptReaderEx(const char *script_path, int *slen)
    {
        // default script name: ./script/c%d.lua
        if (std::strncmp(script_path, "./script", 8) != 0) // not a card script file
            return ReadScriptFromFile(script_path, slen);
        const char *script_name = script_path + 2;
        char expansions_path[1024]{};
        mysnprintf(expansions_path, "./expansions/%s", script_name);
#ifdef YGOPRO_SERVER_MODE
        char special_path[1024]{};
        mysnprintf(special_path, "./specials/%s", script_path + 9);
        if (ReadScriptFromFile(special_path, slen))
            return scriptBuffer;
        if (ReadScriptFromFile(expansions_path, slen)) // always read expansions first
            return scriptBuffer;
#ifdef SERVER_ZIP_SUPPORT
        if (ReadScriptFromIrrFS(script_name, slen))
            return scriptBuffer;
#endif
        if (ReadScriptFromFile(script_path, slen))
            return scriptBuffer;
#else  // YGOPRO_SERVER_MODE
        if (mainGame->gameConf.prefer_expansion_script)
        { // debug script with raw file in expansions
            if (ReadScriptFromFile(expansions_path, slen))
                return scriptBuffer;
            if (ReadScriptFromIrrFS(script_name, slen))
                return scriptBuffer;
            if (ReadScriptFromFile(script_path, slen))
                return scriptBuffer;
        }
        else
        {
            if (ReadScriptFromIrrFS(script_name, slen))
                return scriptBuffer;
            if (ReadScriptFromFile(script_path, slen))
                return scriptBuffer;
            if (ReadScriptFromFile(expansions_path, slen))
                return scriptBuffer;
        }
#endif // YGOPRO_SERVER_MODE
        return nullptr;
    }
#if !defined(YGOPRO_SERVER_MODE) || defined(SERVER_ZIP_SUPPORT)
    unsigned char *DataManager::ReadScriptFromIrrFS(const char *script_name, int *slen)
    {
        auto reader = dataManager.IrrFileSystem->createAndOpenFile(script_name);
        if (!reader)
            return nullptr;
        int size = reader->read(scriptBuffer, sizeof scriptBuffer);
        reader->drop();
        if (size >= (int)sizeof scriptBuffer)
            return nullptr;
        *slen = size;
        return scriptBuffer;
    }
#endif // YGOPRO_SERVER_MODE
    unsigned char *DataManager::ReadScriptFromFile(const char *script_name, int *slen)
    {
        FILE *fp = myfopen(script_name, "rb");
        if (!fp)
            return nullptr;
        size_t len = std::fread(scriptBuffer, 1, sizeof scriptBuffer, fp);
        std::fclose(fp);
        if (len >= sizeof scriptBuffer)
            return nullptr;
        *slen = (int)len;
        return scriptBuffer;
    }
#ifndef YGOPRO_SERVER_MODE
    bool DataManager::deck_sort_lv(const CardDataC *p1, const CardDataC *p2)
    {
        if ((p1->type & 0x7) != (p2->type & 0x7))
            return (p1->type & 0x7) < (p2->type & 0x7);
        if ((p1->type & 0x7) == 1)
        {
            auto type1 = (p1->type & 0x48020c0) ? (p1->type & 0x48020c1) : (p1->type & 0x31);
            auto type2 = (p2->type & 0x48020c0) ? (p2->type & 0x48020c1) : (p2->type & 0x31);
            if (type1 != type2)
                return type1 < type2;
            if (p1->level != p2->level)
                return p1->level > p2->level;
            if (p1->attack != p2->attack)
                return p1->attack > p2->attack;
            if (p1->defense != p2->defense)
                return p1->defense > p2->defense;
            return p1->code < p2->code;
        }
        if ((p1->type & 0xfffffff8) != (p2->type & 0xfffffff8))
            return (p1->type & 0xfffffff8) < (p2->type & 0xfffffff8);
        return p1->code < p2->code;
    }
    bool DataManager::deck_sort_atk(const CardDataC *p1, const CardDataC *p2)
    {
        if ((p1->type & 0x7) != (p2->type & 0x7))
            return (p1->type & 0x7) < (p2->type & 0x7);
        if ((p1->type & 0x7) == 1)
        {
            if (p1->attack != p2->attack)
                return p1->attack > p2->attack;
            if (p1->defense != p2->defense)
                return p1->defense > p2->defense;
            if (p1->level != p2->level)
                return p1->level > p2->level;
            auto type1 = (p1->type & 0x48020c0) ? (p1->type & 0x48020c1) : (p1->type & 0x31);
            auto type2 = (p2->type & 0x48020c0) ? (p2->type & 0x48020c1) : (p2->type & 0x31);
            if (type1 != type2)
                return type1 < type2;
            return p1->code < p2->code;
        }
        if ((p1->type & 0xfffffff8) != (p2->type & 0xfffffff8))
            return (p1->type & 0xfffffff8) < (p2->type & 0xfffffff8);
        return p1->code < p2->code;
    }
    bool DataManager::deck_sort_def(const CardDataC *p1, const CardDataC *p2)
    {
        if ((p1->type & 0x7) != (p2->type & 0x7))
            return (p1->type & 0x7) < (p2->type & 0x7);
        if ((p1->type & 0x7) == 1)
        {
            if (p1->defense != p2->defense)
                return p1->defense > p2->defense;
            if (p1->attack != p2->attack)
                return p1->attack > p2->attack;
            if (p1->level != p2->level)
                return p1->level > p2->level;
            auto type1 = (p1->type & 0x48020c0) ? (p1->type & 0x48020c1) : (p1->type & 0x31);
            auto type2 = (p2->type & 0x48020c0) ? (p2->type & 0x48020c1) : (p2->type & 0x31);
            if (type1 != type2)
                return type1 < type2;
            return p1->code < p2->code;
        }
        if ((p1->type & 0xfffffff8) != (p2->type & 0xfffffff8))
            return (p1->type & 0xfffffff8) < (p2->type & 0xfffffff8);
        return p1->code < p2->code;
    }
    bool DataManager::deck_sort_name(const CardDataC *p1, const CardDataC *p2)
    {
        const wchar_t *name1 = dataManager.GetName(p1->code);
        const wchar_t *name2 = dataManager.GetName(p2->code);
        int res = std::wcscmp(name1, name2);
        if (res != 0)
            return res < 0;
        return p1->code < p2->code;
    }
    bool DataManager::deck_sort_id(const CardDataC *p1, const CardDataC *p2)
    {
        return p1->code < p2->code;
    }
#endif // YGOPRO_SERVER_MODE

}
