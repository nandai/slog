#pragma execution_character_set("utf-8")

#include "slog/Convert.h"
#include "slog/DateTime.h"
#include "slog/Json.h"
#include "slog/Resource.h"
#include "slog/PointerString.h"
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
            void test03();
};

const char* JsonTest::CLS_NAME = "JsonTest";

void JsonTest::run()
{
    test01();
    test02();
    test03();
}

void JsonTest::test01()
{
    SLOG(CLS_NAME, "test01");

    String str;
    Json* json1 = Json::getNewObject();

    json1->serialize(&str);
    SMSG(slog::DEBUG, "%s", str.getBuffer());
    SASSERT("01", str.equals("{}"));

    json1->add("name", "printf");
    json1->serialize(&str);
    SMSG(slog::DEBUG, "%s", str.getBuffer());
    SASSERT("02", str.equals("{\"name\":\"printf\"}"));

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
    SASSERT("01", str.equals(
        "["
            "{\"addr\":\"Tokyo\",\"tel\":\"03\"},"
            "{\"addr\":\"Osaka\",\"tel\":\"06\"}"
        "]"));

    delete json1;
}

void JsonTest::test03()
{
    SLOG(CLS_NAME, "test03");
    String str;

    Json* json1 = Json::getNewObject();
    Json* json2 = Json::getNewObject("messages");

    json1->add(json2);
    json1->add("kind", "0003");

    json1->serialize(&str);
    SMSG(slog::DEBUG, "%s", str.getBuffer());
    SASSERT("01", str.equals("{\"messages\":[],\"kind\":\"0003\"}"));

    delete json1;
}
}

/*!
 * Resourceテスト
 */
namespace slog
{
class R : public Resource
{
            static const LanguageStringList mLanguageStringList[];

public:     static const int32_t greetings = 0;

public:     R(const CoreString* language);
};

static const char* en[] =    {"How do you do"};
static const char* ja[] =    {"はじめまして"};
static const char* zh_TW[] = {"你好嗎"};

const Resource::LanguageStringList R::mLanguageStringList[] =
{
    {"en",    en   },
    {"ja",    ja   },
    {"zh-TW", zh_TW},
};

R::R(const CoreString* language) : Resource(language, mLanguageStringList, sizeof(mLanguageStringList) / sizeof(mLanguageStringList[0]))
{
}

class ResourceTest : public Test
{
            static const char* CLS_NAME;

public:     virtual void run() override;

private:    void test01();
            void test02();
            void test03();
            void test04();
};

const char* ResourceTest::CLS_NAME = "ResourceTest";

void ResourceTest::run()
{
    test01();
    test02();
    test03();
    test04();
}

void ResourceTest::test01()
{
    SLOG(CLS_NAME, "test01");

    PointerString language = "en";
    R r(&language);

    PointerString str = r.string(R::greetings);
    SASSERT("01", str.equals("How do you do"));
}

void ResourceTest::test02()
{
    SLOG(CLS_NAME, "test02");

    PointerString language = "ja";
    R r(&language);

    PointerString str = r.string(R::greetings);
    SASSERT("01", str.equals("はじめまして"));
}

void ResourceTest::test03()
{
    SLOG(CLS_NAME, "test03");

    PointerString language = "zh-TW";
    R r(&language);

    PointerString str = r.string(R::greetings);
    SASSERT("01", str.equals("你好嗎"));
}

void ResourceTest::test04()
{
    SLOG(CLS_NAME, "test04");

    PointerString language = "fr";
    R r(&language);

    PointerString str = r.string(R::greetings);
    SASSERT("01", str.equals("How do you do"));
}
}

/*!
 * Stringテスト
 */
namespace slog
{
class StringTest : public Test
{
            static const char* CLS_NAME;

public:     virtual void run() override;

private:    void test01();
};

const char* StringTest::CLS_NAME = "StringTest";

void StringTest::run()
{
    test01();
}

void StringTest::test01()
{
    SLOG(CLS_NAME, "test01");
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
//  testManager.add(new ConvertTest);
//  testManager.add(new DateTimeTest);
    testManager.add(new JsonTest);
//  testManager.add(new ResourceTest);
//  testManager.add(new StringTest);
    testManager.run();

    return 0;
}
