/* В результате рефакторинга уберем выбросы исключений и заменим на возврат ошибки */

#include <optional>

enum class DateTimeErrorCode {
    OK,
    YEAR,
    MONTH,
    DAY,
    HOUR,
    MINUTE,
    SECOND
};

DateTimeErrorCode CheckDateTimeValidity(const DateTime& dt) {

    if (dt.year < 1 || dt.year > 9999) {
        return DateTimeErrorCode::YEAR;    
    }

    if (dt.month < 1 || dt.month > 12) {
        return DateTimeErrorCode::MONTH;
    }

    const bool is_leap_year = (dt.year % 4 == 0) && !(dt.year % 100 == 0 && dt.year % 400 != 0);
    const array month_lengths = {31, 28 + is_leap_year, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (dt.day < 1 || dt.day > month_lengths[dt.month - 1]) {
        return DateTimeErrorCode::DAY;
    }

    if (dt.hour < 0 || dt.hour > 23) {
        return DateTimeErrorCode::HOUR;
    }

    if (dt.minute < 0 || dt.minute > 59) {
        return DateTimeErrorCode::MINUTE;
    }

    if (dt.second < 0 || dt.second > 59) {
        return DateTimeErrorCode::SECOND;
    }

    return DateTimeErrorCode::OK;
}