#include <stdexcept>
#include <string_view>
#include <optional>

using namespace std;

struct DBParams {
    string_view db_name;
    int db_connection_timeout;
    bool db_allow_exceptions;
    DBLogLevel db_log_level;
};

struct DBParamsBuilder {

    DBParamsBuilder& SetDBName(string_view db_name) {
        this->db_name = db_name;
        settings_flags |= static_cast<SettingsFlags>(SettingsFlag::DB_NAME_SET);
        return *this;
    }

    DBParamsBuilder& SetDBConnectionTimeout(int db_connection_timeout) {
        this->db_connection_timeout = db_connection_timeout;
        settings_flags |= static_cast<SettingsFlags>(SettingsFlag::DB_CONNECTION_TIMEOUT_SET);
        return *this;
    }

    DBParamsBuilder& SetDBAllowExceptions(bool db_allow_exceptions) {
        this->db_allow_exceptions = db_allow_exceptions;
        settings_flags |= static_cast<SettingsFlags>(SettingsFlag::DB_ALLOW_EXCEPTINS_SET);
        return *this;
    }

    DBParamsBuilder& SetDBLogLevel(DBLogLevel db_log_level) {
        this->db_log_level = db_log_level;
        settings_flags |= static_cast<SettingsFlags>(SettingsFlag::DB_LOG_LEVEL_SET);
        return *this;
    }

    operator DBParams() const {
        const SettingsFlags settings_complete = static_cast<SettingsFlags>(SettingsFlag::DB_NAME_SET)
                                                & static_cast<SettingsFlags>(SettingsFlag::DB_CONNECTION_TIMEOUT_SET)
                                                & static_cast<SettingsFlags>(SettingsFlag::DB_ALLOW_EXCEPTINS_SET)
                                                & static_cast<SettingsFlags>(SettingsFlag::DB_LOG_LEVEL_SET);
        if (!(settings_flags & settings_complete)) {
            throw std::logic_error("db setting not complete."s);
        }
        return {db_name, db_connection_timeout, db_allow_exceptions, db_log_level};
    }

private:
    string_view db_name;
    int db_connection_timeout;
    bool db_allow_exceptions;
    DBLogLevel db_log_level;

    enum class SettingsFlag {
        DB_NAME_SET = 1 << 0,
        DB_CONNECTION_TIMEOUT_SET = 1 << 1,
        DB_ALLOW_EXCEPTINS_SET = 1 << 2,
        DB_LOG_LEVEL_SET = 1 << 3
    };

    using SettingsFlags = uint64_t;

    SettingsFlags settings_flags = 0;
};

optional<DBHandler> GetDB(const DBParams& db_params) {
    DBConnector connector(db_params.db_allow_exceptions, db_params.db_log_level);
    DBHandler db;
    if (db_params.db_name.starts_with("tmp."s)) {
        db = connector.ConnectTmp(db_params.db_name, db_params.db_connection_timeout);
    } else {
        db = connector.Connect(db_params.db_name, db_params.db_connection_timeout);
    }
    if (!db_params.db_allow_exceptions && !db.IsOK()) {
        return {};
    }
    return db;
}

/*  Функция получет данные о пользователях через сформированный ей SQL запрос
    с заданными фильтрами: min_age, max_age, name_filter (фильтр по именам) */
/*  Пример вызова функции:

    DBHandler db;
    if (auto get_db = GetDB(DBParamsBuilder().
                            SetDBName(db_name).
                            SetDBConnectionTimeout(db_connection_timeout).
                            SetDBAllowExceptions(db_allow_exceptions).
                            SetDBLogLevel(db_log_level))) {

        vector<Person> persons = LoadPersons(18, 65, "A%", *get_db);
    }

*/
vector<Person> LoadPersons(int min_age, int max_age, string_view name_filter, const DBHandler& db) {

    ostringstream query_str;
    query_str << "from Persons "s
              << "select Name, Age "s
              << "where Age between "s << min_age << " and "s << max_age << " "s
              << "and Name like '%"s << db.Quote(name_filter) << "%'"s;
    DBQuery query(query_str.str());

    vector<Person> persons;
    for (auto [name, age] : db.LoadRows<string, int>(query)) {
        persons.push_back({move(name), age});
    }
    return persons;
}