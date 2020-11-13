#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCaller.h>

class MqttLinkTest : public CppUnit::TestFixture {
public:
    void setUp() {
    }

    void tearDown() {
    }

    void testInitial() {
        CPPUNIT_FAIL("Not implemented yet");
    }

    static auto suite() -> CppUnit::Test*
    {
        CppUnit::TestSuite* suite { new CppUnit::TestSuite{ "MqttLinkTest" } };

        suite->addTest( new CppUnit::TestCaller<MqttLinkTest>{ "testInitial", &MqttLinkTest::testInitial } );

        return suite;
    }
};


auto main() -> int
{
    CppUnit::TextUi::TestRunner runner {};
    runner.addTest( MqttLinkTest::suite() );

    return (runner.run("", false))?0:1;
}
