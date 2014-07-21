﻿#pragma execution_character_set("utf-8")

#include "slog/Convert.h"
#include "slog/DateTime.h"
#include "slog/Json.h"
#include "slog/SequenceLog.h"

#include <vector>

using namespace slog;

class Test
{
public:     virtual void run() = 0;
};

class TestManager
{
            std::vector<Test*> mTests;

public:     ~TestManager();

            void add(Test* test);
            void run();
};

TestManager::~TestManager()
{
    for (auto  test : mTests)
        delete test;

    mTests.clear();
}

void TestManager::add(Test* test)
{
    mTests.push_back(test);
}

void TestManager::run()
{
    for (auto test : mTests)
        test->run();
}

/*!
 * Convertテスト
 */
namespace slog
{
class ConvertTest : public Test
{
            static const char* CLS_NAME;

public:     virtual void run() override;

private:    void test01();
            void test02();
            void test03();
};

const char* ConvertTest::CLS_NAME = "ConvertTest";

void ConvertTest::run()
{
    test01();
    test02();
    test03();
}

void ConvertTest::test01()
{
    SLOG(CLS_NAME, "test01");
    int8_t value;

    value = Convert::toByte("");
    SASSERT("01", value == 0);

    value = Convert::toByte("0");
    SASSERT("02", value == 0);

    value = Convert::toByte("@1");
    SASSERT("03", value == 0);

    value = Convert::toByte("1@");
    SASSERT("04", value == 1);

    value = Convert::toByte("-128");
    SASSERT("05", value == -128);

    value = Convert::toByte("128");
    SASSERT("06", value == -128);

    value = Convert::toByte("255");
    SASSERT("07", value == -1);

    value = Convert::toByte("256");
    SASSERT("08", value == 0);

    value = Convert::toByte("123", 10, 2);
    SASSERT("09", value == 12);
}

void ConvertTest::test02()
{
    SLOG(CLS_NAME, "test02");
    int32_t value;

    value = Convert::toInt("");
    SASSERT("01", value == 0);

    value = Convert::toInt("0");
    SASSERT("02", value == 0);

    value = Convert::toInt("@1");
    SASSERT("03", value == 0);

    value = Convert::toInt("1@");
    SASSERT("04", value == 1);

    value = Convert::toInt("-128");
    SASSERT("05", value == -128);

    value = Convert::toInt("128");
    SASSERT("06", value == 128);

    value = Convert::toInt("255");
    SASSERT("07", value == 255);

    value = Convert::toInt("256");
    SASSERT("08", value == 256);

    value = Convert::toInt("123", 10, 2);
    SASSERT("09", value == 12);
}

void ConvertTest::test03()
{
    SLOG(CLS_NAME, "test03");
    int8_t value;

    value = Convert::toByte("", 16);
    SASSERT("01", value == 0x00);

    value = Convert::toByte("0", 16);
    SASSERT("02", value == 0x00);

    value = Convert::toByte("@a", 16);
    SASSERT("03", value == 0x00);

    value = Convert::toByte("a@", 16);
    SASSERT("04", value == 0x0A);

    value = Convert::toByte("80", 16);
    SASSERT("05", value == (int8_t)0x80);

    value = Convert::toByte("FF", 16);
    SASSERT("06", value == (int8_t)0xFF);

    value = Convert::toByte("100", 16);
    SASSERT("07", value == 0x00);

    value = Convert::toByte("100", 16, 2);
    SASSERT("08", value == 0x10);
}
}

/*!
 * DateTimeテスト
 */
namespace slog
{
class DateTimeTest : public Test
{
            static const char* CLS_NAME;

public:     virtual void run() override;

private:    void test01();
            void test02();

            void output(const char* title,const DateTime* dateTime);
};

const char* DateTimeTest::CLS_NAME = "DateTimeTest";

void DateTimeTest::run()
{
    test01();
    test02();
}

void DateTimeTest::test01()
{
    SLOG(CLS_NAME, "test01");
    DateTime dateTime;

    dateTime.setCurrent();
    output("現在日時(UTC)  ", &dateTime);

    dateTime.toLocal();
    output("現在日時(LOCAL)", &dateTime);
}

void DateTimeTest::test02()
{
    SLOG(CLS_NAME, "test02");
    DateTime dateTime;

    dateTime.setYear(1899);
    SASSERT("01", dateTime.getYear() == 2155);

    dateTime.setYear(1900);
    SASSERT("02", dateTime.getYear() == 1900);

    dateTime.setYear(2155);
    SASSERT("03", dateTime.getYear() == 2155);

    dateTime.setYear(2156);
    SASSERT("04", dateTime.getYear() == 1900);

    dateTime.setYear(2014);
    dateTime.setMonth(7);
    dateTime.setDay(6);
    SASSERT("05", dateTime.getWeekDay() == 0);  // 日曜日
}

void DateTimeTest::output(const char* title,const DateTime* dateTime)
{
    SLOG(CLS_NAME, "output");
    SMSG(slog::DEBUG, "%s:%04d/%02d/%02d %02d:%02d:%02d.%03d",
        title,
        dateTime->getYear(),
        dateTime->getMonth(),
        dateTime->getDay(),
        dateTime->getHour(),
        dateTime->getMinute(),
        dateTime->getSecond(),
        dateTime->getMilliSecond());
}
}

/*!
 * Jsonテスト
 */
namespace slog
{
class JsonTest : public Test
{
            static const char* CLS_NAME;

public:     virtual void run() override;

private:    void test01();
            void test02();
};

const char* JsonTest::CLS_NAME = "JsonTest";

void JsonTest::run()
{
    test01();
    test02();
}

void JsonTest::test01()
{
    SLOG(CLS_NAME, "test01");

    String str;
    Json* json1 = Json::getNewObject();

    json1->serialize(&str);
    SMSG(slog::DEBUG, "%s", str.getBuffer());

    json1->add("name", "printf");
    json1->serialize(&str);
    SMSG(slog::DEBUG, "%s", str.getBuffer());

    delete json1;
}

void JsonTest::test02()
{
    SLOG(CLS_NAME, "test01");

    String str;
    Json* json1 = Json::getNewObject();

    Json* json2 = Json::getNewObject();
    json2->add("addr", "Tokyo");
    json2->add("tel", "03");
    json1->add(json2);

    Json* json3 = Json::getNewObject();
    json3->add("addr", "Osaka");
    json3->add("tel", "06");
    json1->add(json3);

    json1->serialize(&str);
    SMSG(slog::DEBUG, "%s", str.getBuffer());

    delete json1;
}
}

/*!
 * \brief   メイン
 */
int main()
{
#if defined(_WINDOWS)
    loadSequenceLogConfig("../../../src/SequenceLogLib/test/test.log.config");
#else
    loadSequenceLogConfig("test.log.config");
#endif

//  SLOG("test.cpp", "main");

    TestManager testManager;
    testManager.add(new ConvertTest);
    testManager.add(new DateTimeTest);
    testManager.add(new JsonTest);
    testManager.run();

    return 0;
}
