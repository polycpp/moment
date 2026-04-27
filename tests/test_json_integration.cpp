/**
 * @file test_json_integration.cpp
 * @brief Tests for JsonValue/JsonObject/JsonArray integration with Moment and Duration.
 *
 * Tests toObject(), toArray() as JsonArray, fromObject(), utcFromObject(),
 * Duration(JsonObject), duration(JsonObject), and round-trip conversions.
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>
#include <polycpp/core/json.hpp>
#include <limits>

using namespace polycpp::moment;
using polycpp::JsonObject;
using polycpp::JsonArray;
using polycpp::JsonValue;
using polycpp::Date;
namespace JSON = polycpp::JSON;

// ═════════════════════════════════════════════════════════════════════
// Moment::toObject
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, MomentToObjectReturnsCorrectKeys) {
    // 2024-03-15T14:30:45.123Z
    auto m = utcFromMs(1710513045123LL);
    auto obj = m.toObject();
    EXPECT_EQ(obj.at("years").asInt(), 2024);
    EXPECT_EQ(obj.at("months").asInt(), 2);     // 0-based
    EXPECT_EQ(obj.at("date").asInt(), 15);
    EXPECT_EQ(obj.at("hours").asInt(), 14);
    EXPECT_EQ(obj.at("minutes").asInt(), 30);
    EXPECT_EQ(obj.at("seconds").asInt(), 45);
    EXPECT_EQ(obj.at("milliseconds").asInt(), 123);
    EXPECT_EQ(obj.size(), 7u);
}

TEST(JsonIntegrationTest, MomentToObjectHasExactKeys) {
    auto m = utcFromMs(0LL);
    auto obj = m.toObject();
    // Verify the exact key names match moment.js
    EXPECT_TRUE(obj.count("years"));
    EXPECT_TRUE(obj.count("months"));
    EXPECT_TRUE(obj.count("date"));
    EXPECT_TRUE(obj.count("hours"));
    EXPECT_TRUE(obj.count("minutes"));
    EXPECT_TRUE(obj.count("seconds"));
    EXPECT_TRUE(obj.count("milliseconds"));
    // Must NOT have these keys
    EXPECT_FALSE(obj.count("year"));
    EXPECT_FALSE(obj.count("month"));
    EXPECT_FALSE(obj.count("day"));
}

TEST(JsonIntegrationTest, MomentToObjectEpoch) {
    auto m = utcFromMs(0LL);
    auto obj = m.toObject();
    EXPECT_EQ(obj.at("years").asInt(), 1970);
    EXPECT_EQ(obj.at("months").asInt(), 0);
    EXPECT_EQ(obj.at("date").asInt(), 1);
    EXPECT_EQ(obj.at("hours").asInt(), 0);
    EXPECT_EQ(obj.at("minutes").asInt(), 0);
    EXPECT_EQ(obj.at("seconds").asInt(), 0);
    EXPECT_EQ(obj.at("milliseconds").asInt(), 0);
}

TEST(JsonIntegrationTest, MomentToObjectEndOfYear) {
    // 2024-12-31T23:59:59.999Z
    auto m = utcFromDate(2024, 11, 31, 23, 59, 59, 999);
    auto obj = m.toObject();
    EXPECT_EQ(obj.at("years").asInt(), 2024);
    EXPECT_EQ(obj.at("months").asInt(), 11);
    EXPECT_EQ(obj.at("date").asInt(), 31);
    EXPECT_EQ(obj.at("hours").asInt(), 23);
    EXPECT_EQ(obj.at("minutes").asInt(), 59);
    EXPECT_EQ(obj.at("seconds").asInt(), 59);
    EXPECT_EQ(obj.at("milliseconds").asInt(), 999);
}

TEST(JsonIntegrationTest, MomentToObjectInvalidMoment) {
    auto m = invalid();
    auto obj = m.toObject();
    // Invalid moment still returns components (7 keys)
    EXPECT_EQ(obj.size(), 7u);
}

// ═════════════════════════════════════════════════════════════════════
// Moment::toArray (JsonArray)
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, MomentToArrayReturnsJsonArray) {
    auto m = utcFromMs(1710513045123LL);
    auto arr = m.toArray();
    EXPECT_EQ(arr.size(), 7u);
    EXPECT_EQ(arr[0].asInt(), 2024);
    EXPECT_EQ(arr[1].asInt(), 2);
    EXPECT_EQ(arr[2].asInt(), 15);
    EXPECT_EQ(arr[3].asInt(), 14);
    EXPECT_EQ(arr[4].asInt(), 30);
    EXPECT_EQ(arr[5].asInt(), 45);
    EXPECT_EQ(arr[6].asInt(), 123);
}

TEST(JsonIntegrationTest, MomentToArrayElementsAreNumbers) {
    auto m = utcFromMs(0LL);
    auto arr = m.toArray();
    for (size_t i = 0; i < arr.size(); ++i) {
        EXPECT_TRUE(arr[i].isNumber()) << "Element " << i << " should be a number";
    }
}

// ═════════════════════════════════════════════════════════════════════
// fromObject
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, FromObjectWithAllFields) {
    auto m = fromObject(JsonObject{
        {"year", 2024}, {"month", 2}, {"date", 15},
        {"hour", 14}, {"minute", 30}, {"second", 45}, {"millisecond", 123}
    });
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.second(), 45);
    EXPECT_EQ(m.millisecond(), 123);
}

TEST(JsonIntegrationTest, FromObjectWithPartialFields) {
    auto m = fromObject(JsonObject{{"year", 2024}, {"month", 5}});
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 5);
    EXPECT_EQ(m.date(), 1);    // default
    EXPECT_EQ(m.hour(), 0);    // default
    EXPECT_EQ(m.minute(), 0);  // default
    EXPECT_EQ(m.second(), 0);  // default
    EXPECT_EQ(m.millisecond(), 0); // default
}

TEST(JsonIntegrationTest, FromObjectAcceptsPluralKeys) {
    auto m = fromObject(JsonObject{
        {"years", 2024}, {"months", 3}
    });
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 3);
}

TEST(JsonIntegrationTest, FromObjectSingularKeyTakesPrecedence) {
    // When both singular and plural present, singular wins
    auto m = fromObject(JsonObject{
        {"year", 2024}, {"years", 1999}
    });
    EXPECT_EQ(m.year(), 2024);
}

TEST(JsonIntegrationTest, FromObjectDateKeyAlternative) {
    // "date" is the primary key for day-of-month, "day" is the fallback
    auto m1 = fromObject(JsonObject{{"year", 2024}, {"month", 0}, {"date", 15}});
    EXPECT_EQ(m1.date(), 15);

    auto m2 = fromObject(JsonObject{{"year", 2024}, {"month", 0}, {"day", 20}});
    EXPECT_EQ(m2.date(), 20);
}

TEST(JsonIntegrationTest, FromObjectEmptyObjectUsesDefaults) {
    auto m = fromObject(JsonObject{});
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2000);
    EXPECT_EQ(m.month(), 0);
    EXPECT_EQ(m.date(), 1);
    EXPECT_EQ(m.hour(), 0);
}

TEST(JsonIntegrationTest, UtcFromObjectCreatesUtcMoment) {
    auto m = utcFromObject(JsonObject{
        {"year", 2024}, {"month", 0}, {"date", 1}
    });
    EXPECT_TRUE(m.isUtc());
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 0);
    EXPECT_EQ(m.date(), 1);
}

TEST(JsonIntegrationTest, UtcFromObjectWithTime) {
    auto m = utcFromObject(JsonObject{
        {"year", 2024}, {"month", 2}, {"date", 15},
        {"hour", 14}, {"minute", 30}
    });
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
}

TEST(JsonIntegrationTest, FromObjectAcceptsJsonValueObject) {
    JsonValue value(JsonObject{
        {"year", 2024}, {"month", 2}, {"date", 15},
        {"hour", 14}, {"minute", 30}
    });
    auto m = fromObject(value);
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
}

TEST(JsonIntegrationTest, UtcFromObjectAcceptsJsonValueObject) {
    JsonValue value(JsonObject{{"years", 2024}, {"months", 0}, {"date", 1}});
    auto m = utcFromObject(value);
    EXPECT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 0);
    EXPECT_EQ(m.date(), 1);
}

TEST(JsonIntegrationTest, FromObjectRejectsNonObjectJsonValue) {
    auto m = fromObject(JsonValue("2024-03-15"));
    EXPECT_FALSE(m.isValid());
    EXPECT_TRUE(m.parsingFlags().invalidFormat);

    auto nullMoment = utcFromObject(JsonValue(nullptr));
    EXPECT_FALSE(nullMoment.isValid());
    EXPECT_TRUE(nullMoment.isUtc());
    EXPECT_TRUE(nullMoment.parsingFlags().nullInput);
}

// ═════════════════════════════════════════════════════════════════════
// polycpp::Date interop
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, FromDateAcceptsPolycppDate) {
    Date date(1710513045123.0);
    auto m = fromDate(date);
    EXPECT_TRUE(m.isValid());
    EXPECT_TRUE(m.isLocal());
    EXPECT_EQ(m.valueOf(), 1710513045123LL);
}

TEST(JsonIntegrationTest, UtcFromDateAcceptsPolycppDate) {
    Date date(1710513045123.0);
    auto m = utcFromDate(date);
    EXPECT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.valueOf(), 1710513045123LL);
}

TEST(JsonIntegrationTest, FromDateRejectsInvalidPolycppDate) {
    Date date(std::numeric_limits<double>::quiet_NaN());
    auto m = fromDate(date);
    EXPECT_FALSE(m.isValid());
    EXPECT_TRUE(m.parsingFlags().invalidFormat);
}

// ═════════════════════════════════════════════════════════════════════
// Moment toObject/fromObject round-trip
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, MomentToObjectFromObjectRoundTrip) {
    auto m1 = utcFromMs(1710513045123LL);
    auto obj = m1.toObject();
    // toObject uses "years" and "date" keys — fromObject accepts them
    auto m2 = utcFromObject(obj);
    EXPECT_EQ(m1.valueOf(), m2.valueOf());
}

TEST(JsonIntegrationTest, MomentToObjectFromObjectRoundTripEpoch) {
    auto m1 = utcFromMs(0LL);
    auto obj = m1.toObject();
    auto m2 = utcFromObject(obj);
    EXPECT_EQ(m1.valueOf(), m2.valueOf());
}

// ═════════════════════════════════════════════════════════════════════
// Duration::toObject
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, DurationToObjectReturnsCorrectKeys) {
    auto d = duration(DurationInput{
        .years = 1, .months = 2, .days = 3,
        .hours = 4, .minutes = 5, .seconds = 6, .milliseconds = 7
    });
    auto obj = d.toObject();
    EXPECT_EQ(obj.at("years").asInt(), 1);
    EXPECT_EQ(obj.at("months").asInt(), 2);
    EXPECT_EQ(obj.at("days").asInt(), 3);
    EXPECT_EQ(obj.at("hours").asInt(), 4);
    EXPECT_EQ(obj.at("minutes").asInt(), 5);
    EXPECT_EQ(obj.at("seconds").asInt(), 6);
    EXPECT_EQ(obj.at("milliseconds").asInt(), 7);
    EXPECT_EQ(obj.size(), 7u);
}

TEST(JsonIntegrationTest, DurationToObjectZeroDuration) {
    auto d = duration(static_cast<int64_t>(0));
    auto obj = d.toObject();
    EXPECT_EQ(obj.at("years").asInt(), 0);
    EXPECT_EQ(obj.at("months").asInt(), 0);
    EXPECT_EQ(obj.at("days").asInt(), 0);
    EXPECT_EQ(obj.at("hours").asInt(), 0);
    EXPECT_EQ(obj.at("minutes").asInt(), 0);
    EXPECT_EQ(obj.at("seconds").asInt(), 0);
    EXPECT_EQ(obj.at("milliseconds").asInt(), 0);
}

TEST(JsonIntegrationTest, DurationToObjectFromMilliseconds) {
    // 1 hour, 30 minutes, 45 seconds, 500 ms = 5445500 ms
    auto d = duration(static_cast<int64_t>(5445500));
    auto obj = d.toObject();
    EXPECT_EQ(obj.at("hours").asInt(), 1);
    EXPECT_EQ(obj.at("minutes").asInt(), 30);
    EXPECT_EQ(obj.at("seconds").asInt(), 45);
    EXPECT_EQ(obj.at("milliseconds").asInt(), 500);
}

// ═════════════════════════════════════════════════════════════════════
// Duration from JsonObject
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, DurationFromJsonObjectAllFields) {
    auto d = duration(JsonObject{
        {"years", 1}, {"months", 2}, {"weeks", 1}, {"days", 3},
        {"hours", 4}, {"minutes", 5}, {"seconds", 6}, {"milliseconds", 7}
    });
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.years(), 1);
    EXPECT_EQ(d.months(), 2);
    EXPECT_EQ(d.days(), 10);   // 3 + 1*7 = 10
    EXPECT_EQ(d.hours(), 4);
    EXPECT_EQ(d.minutes(), 5);
    EXPECT_EQ(d.seconds(), 6);
    EXPECT_EQ(d.milliseconds(), 7);
}

TEST(JsonIntegrationTest, DurationFromJsonObjectPartialFields) {
    auto d = duration(JsonObject{{"hours", 2}, {"minutes", 30}});
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.hours(), 2);
    EXPECT_EQ(d.minutes(), 30);
    EXPECT_EQ(d.years(), 0);
    EXPECT_EQ(d.months(), 0);
    EXPECT_EQ(d.days(), 0);
    EXPECT_EQ(d.seconds(), 0);
    EXPECT_EQ(d.milliseconds(), 0);
}

TEST(JsonIntegrationTest, DurationFromJsonObjectEmptyObject) {
    auto d = duration(JsonObject{});
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.asMilliseconds(), 0);
}

TEST(JsonIntegrationTest, DurationFromJsonObjectMatchesDurationInput) {
    auto d1 = duration(DurationInput{.years = 2, .months = 3, .days = 5, .hours = 10});
    auto d2 = duration(JsonObject{{"years", 2}, {"months", 3}, {"days", 5}, {"hours", 10}});
    EXPECT_EQ(d1.asMilliseconds(), d2.asMilliseconds());
    EXPECT_EQ(d1.years(), d2.years());
    EXPECT_EQ(d1.months(), d2.months());
    EXPECT_EQ(d1.days(), d2.days());
    EXPECT_EQ(d1.hours(), d2.hours());
}

TEST(JsonIntegrationTest, DurationFromJsonObjectWeeksConversion) {
    auto d = duration(JsonObject{{"weeks", 3}});
    EXPECT_EQ(d.days(), 21);
    EXPECT_EQ(d.weeks(), 3);
}

TEST(JsonIntegrationTest, DurationFromJsonObjectIgnoresUnknownKeys) {
    auto d = duration(JsonObject{{"hours", 5}, {"unknown_key", 999}});
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.hours(), 5);
}

TEST(JsonIntegrationTest, DurationFromJsonValueObject) {
    JsonValue value(JsonObject{{"hours", 2}, {"minutes", 30}});
    auto d = duration(value);
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.hours(), 2);
    EXPECT_EQ(d.minutes(), 30);
}

TEST(JsonIntegrationTest, DurationConstructorFromJsonValueObject) {
    Duration d(JsonValue(JsonObject{{"weeks", 1}, {"days", 2}}));
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.days(), 9);
}

TEST(JsonIntegrationTest, DurationFromJsonValueRejectsNonObject) {
    auto d = duration(JsonValue("PT2H"));
    EXPECT_FALSE(d.isValid());
}

// ═════════════════════════════════════════════════════════════════════
// Duration toObject/fromObject round-trip
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, DurationToObjectFromObjectRoundTrip) {
    auto d1 = duration(DurationInput{.years = 1, .months = 6, .days = 15, .hours = 12});
    auto obj = d1.toObject();
    auto d2 = duration(obj);
    EXPECT_EQ(d1.years(), d2.years());
    EXPECT_EQ(d1.months(), d2.months());
    EXPECT_EQ(d1.days(), d2.days());
    EXPECT_EQ(d1.hours(), d2.hours());
    EXPECT_EQ(d1.minutes(), d2.minutes());
    EXPECT_EQ(d1.seconds(), d2.seconds());
    EXPECT_EQ(d1.milliseconds(), d2.milliseconds());
}

TEST(JsonIntegrationTest, DurationToObjectFromObjectRoundTripMs) {
    auto d1 = duration(static_cast<int64_t>(86400000 + 3600000 + 60000 + 1000 + 500));
    auto obj = d1.toObject();
    auto d2 = duration(obj);
    EXPECT_EQ(d1.days(), d2.days());
    EXPECT_EQ(d1.hours(), d2.hours());
    EXPECT_EQ(d1.minutes(), d2.minutes());
    EXPECT_EQ(d1.seconds(), d2.seconds());
    EXPECT_EQ(d1.milliseconds(), d2.milliseconds());
}

// ═════════════════════════════════════════════════════════════════════
// JSON serialization interop
// ═════════════════════════════════════════════════════════════════════

TEST(JsonIntegrationTest, MomentToObjectSerializesToJson) {
    auto m = utcFromMs(1710513045123LL);
    auto obj = m.toObject();
    auto json = JSON::stringify(JsonValue(obj));
    EXPECT_FALSE(json.empty());
    // Parse back and verify a field
    auto parsed = JSON::parse(json);
    EXPECT_EQ(parsed["years"].asInt(), 2024);
}

TEST(JsonIntegrationTest, DurationToObjectSerializesToJson) {
    auto d = duration(DurationInput{.years = 2, .months = 6});
    auto obj = d.toObject();
    auto json = JSON::stringify(JsonValue(obj));
    EXPECT_FALSE(json.empty());
    auto parsed = JSON::parse(json);
    EXPECT_EQ(parsed["years"].asInt(), 2);
    EXPECT_EQ(parsed["months"].asInt(), 6);
}

TEST(JsonIntegrationTest, MomentToArraySerializesToJson) {
    auto m = utcFromMs(1710513045123LL);
    auto arr = m.toArray();
    auto json = JSON::stringify(JsonValue(arr));
    EXPECT_FALSE(json.empty());
    auto parsed = JSON::parse(json);
    EXPECT_TRUE(parsed.isArray());
    EXPECT_EQ(parsed.asArray().size(), 7u);
    EXPECT_EQ(parsed[0].asInt(), 2024);
}

TEST(JsonIntegrationTest, MomentSerializesDirectlyWithJsonStringify) {
    auto m = utcFromMs(1710513045123LL);
    EXPECT_EQ(JSON::stringify(m), "\"" + m.toJSON() + "\"");
}

TEST(JsonIntegrationTest, DurationSerializesDirectlyWithJsonStringify) {
    auto d = duration(DurationInput{.hours = 2, .minutes = 30});
    EXPECT_EQ(JSON::stringify(d), "\"" + d.toJSON() + "\"");
}

TEST(JsonIntegrationTest, ParsingFlagsConvertToJsonObject) {
    auto m = parse("2024-03-15 extra", "YYYY-MM-DD", true);
    auto flags = m.parsingFlags();
    auto obj = flags.toObject();

    EXPECT_EQ(obj.at("charsLeftOver").asInt(), 6);
    EXPECT_TRUE(obj.at("unusedTokens").isArray());
    EXPECT_TRUE(obj.at("unusedInput").isArray());
    EXPECT_TRUE(obj.at("parsedDateParts").isArray());
    EXPECT_TRUE(obj.at("invalidMonth").isNull());
    EXPECT_TRUE(flags.toJSON().isObject());
}

TEST(JsonIntegrationTest, CreationDataConvertToJsonObject) {
    auto m = parse("2024-03-15 extra", "YYYY-MM-DD", true);
    auto data = m.creationData();
    auto obj = data.toObject();

    EXPECT_EQ(obj.at("input").asString(), "2024-03-15 extra");
    EXPECT_EQ(obj.at("format").asString(), "YYYY-MM-DD");
    EXPECT_FALSE(obj.at("isUTC").asBool());
    EXPECT_TRUE(obj.at("strict").asBool());
    EXPECT_TRUE(data.toJSON().isObject());
}

TEST(JsonIntegrationTest, DiagnosticObjectsSerializeDirectlyWithJsonStringify) {
    auto m = parse("2024-03-15 extra", "YYYY-MM-DD", true);
    auto flagsJson = JSON::stringify(m.parsingFlags());
    auto dataJson = JSON::stringify(m.creationData());

    EXPECT_TRUE(JSON::parse(flagsJson).isObject());
    EXPECT_EQ(JSON::parse(dataJson)["format"].asString(), "YYYY-MM-DD");
}
