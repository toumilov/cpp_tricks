
#include "cjson.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <limits>


int main( int argc, char** argv)
{
	return CommandLineTestRunner::RunAllTests( argc, argv );
}

TEST_GROUP(JsonGroup)
{
	void setup()
	{
	}
	void teardown()
	{
	}
};

TEST(JsonGroup, ConstructorTest)
{
	// Default c-tor
	Json empty;
	CHECK( empty.type() == Json::Null );

	// Type c-tor
	{
		Json jb( Json::Bool );
		CHECK( jb.is_bool() );

		Json jn( Json::Number );
		CHECK( jn.is_number() );

		Json js( Json::String );
		CHECK( js.is_string() );

		Json ja( Json::Array );
		CHECK( ja.is_array() );

		Json jo( Json::Object );
		CHECK( jo.is_object() );
	}

	// Boolean c-tor
	Json jb( true );
	CHECK( jb.is_bool() );

	// Number c-tor
	Json jd( 0.5 );
	CHECK( jd.is_number() );

	Json ji( 5 );
	CHECK( ji.is_number() );

	Json ju( 5u );
	CHECK( ju.is_number() );

	// String c-tor
	Json jcs( "test" );
	CHECK( jcs.is_string() );

	Json js( std::string( "test" ) );
	CHECK( js.is_string() );

	// Array c-tor
	Json ja( { 1, 2, 3 } );
	CHECK( ja.is_array() );

	std::vector<std::string> v = { "4", "5", "6" };
	Json jv( v );
	CHECK( jv.is_array() );

	// Copy c-tor
	Json jc( jv );
	CHECK( jc.is_array() );

	// Move c-tor
	Json jm( std::move( js ) );
	CHECK( jm.is_string() );
}

TEST(JsonGroup, Swap)
{
	Json ji( 5 );
	Json js( "test" );
	ji.swap( js );
	CHECK( js.is_number() );
	CHECK( ji.is_string() );
}

TEST(JsonGroup, Assignment)
{
	// Copy assignment
	Json ji( 5 );
	Json o;
	o = ji;
	CHECK( o.is_number() );
	CHECK( ji.is_number() );

	// String assignment
	o = "test";
	CHECK( o.is_string() );

	// Bool assignment
	o = true;
	CHECK( o.is_bool() );

	// Number assignment
	int i = 123;
	o = i;
	CHECK( o.is_number() );
	int32_t i32 = 123;
	o = i32;
	CHECK( o.is_number() );
	int64_t i64 = 123;
	o = i64;
	CHECK( o.is_number() );
	uint32_t u32 = 123;
	o = u32;
	CHECK( o.is_number() );
	uint64_t u64 = 123;
	o = u64;
	CHECK( o.is_number() );
	o = 1.23f;
	CHECK( o.is_number() );
	o = 1.23;
	CHECK( o.is_number() );
}

TEST(JsonGroup, Comparison)
{
	// Bool
	Json jt( true );
	CHECK( jt == true );
	Json jf( false );
	CHECK( jf == false );

	// String
	Json jc( "C string" );
	CHECK( jc == "C string" );
	Json js( "string" );
	CHECK( js == "string" );

	// Number
	Json jn( 5 );
	CHECK( jn == 5 );
	CHECK( jn == (int32_t)5 );
	CHECK( jn == (int64_t)5 );
	CHECK( jn == (uint32_t)5 );
	CHECK( jn == (uint64_t)5 );
	CHECK( jn == 5.0f );
	CHECK( jn == 5.0 );

	// Array
	Json ja( { 1, 2, 3 } );
	std::vector<int> v = { 1, 2, 3 };
	Json jv( v );
	CHECK( ja == jv );

	// Object
	Json o1, o2, o3, o4, o5;
	CHECK( Json::parse( "{\"a\":true,\"b\":1}", o1 ) );
	CHECK( Json::parse( "{\"b\" : 1, \"a\" : true}", o2 ) );
	CHECK( Json::parse( "{\"a\": true}", o3 ) );
	CHECK( Json::parse( "{\"a\":true,\"b\":2}", o4 ) );
	CHECK( Json::parse( "{\"a\":true,\"b\":1,\"c\":\"test\"}", o5 ) );
	CHECK( o1 == o2 );
	CHECK( o1 != o3 );
	CHECK( o1 != o4 );
	CHECK( o1 != o5 );
}

TEST(JsonGroup, OperatorBool)
{
	Json js;
	CHECK( js.operator bool() );
	Json jn( 5 );
	CHECK( !jn );
}

TEST(JsonGroup, Clean)
{
	Json jn( 5 );
	jn.clean();
	CHECK( jn.operator bool() );
}

TEST(JsonGroup, ArrayIndex)
{
	Json ja( { 1, 2 } );
	CHECK_EQUAL( 2, ja.size() );
	CHECK( ja[0].is( Json::Number ) );
	CHECK( ja[1].is( Json::Number ) );
	CHECK( ja[2].is( Json::Null ) );
	CHECK( ja.at( 0 ).is( Json::Number ) );
	CHECK( ja.at( 1 ).is( Json::Number ) );
	CHECK( ja.at( 2 ).is( Json::Null ) );
}

TEST(JsonGroup, ObjectIndex)
{
	Json jo;
	CHECK( Json::parse( "{\"a\":true,\"b\":1,\"c\":\"test\"}", jo ) );

	CHECK( jo["a"].is( Json::Bool ) );
	CHECK( jo["b"].is( Json::Number ) );
	CHECK( jo[std::string( "c" )].is( Json::String ) );
	CHECK( jo["d"].is( Json::Null ) );
	CHECK( jo.at( "a" ).is( Json::Bool ) );
	CHECK( jo.at( "b" ).is( Json::Number ) );
	CHECK( jo.at( std::string( "c" ) ).is( Json::String ) );
	CHECK( jo.at( "d" ).is( Json::Null ) );
}

TEST(JsonGroup, Type)
{
	Json none;
	CHECK_EQUAL( (int)Json::Null, (int)none.type() );
	CHECK( none.is( Json::Null ) );
	CHECK( none.is_null() );

	Json jb( Json::Bool );
	CHECK_EQUAL( (int)Json::Bool, (int)jb.type() );
	CHECK( jb.is( Json::Bool ) );
	CHECK( jb.is_bool() );

	Json jn( Json::Number );
	CHECK_EQUAL( (int)Json::Number, (int)jn.type() );
	CHECK( jn.is( Json::Number ) );
	CHECK( jn.is_number() );

	Json js( Json::String );
	CHECK_EQUAL( (int)Json::String, (int)js.type() );
	CHECK( js.is( Json::String ) );
	CHECK( js.is_string() );

	Json ja( Json::Array );
	CHECK_EQUAL( (int)Json::Array, (int)ja.type() );
	CHECK( ja.is( Json::Array ) );
	CHECK( ja.is_array() );

	Json jo( Json::Object );
	CHECK_EQUAL( (int)Json::Object, (int)jo.type() );
	CHECK( jo.is( Json::Object ) );
	CHECK( jo.is_object() );
}

TEST(JsonGroup, ObjectName)
{
	Json jo;
	CHECK( Json::parse( "{\"a\":true,\"b\":1,\"c\":\"test\"}", jo ) );

	STRCMP_EQUAL( "a", jo["a"].name().c_str() );
	STRCMP_EQUAL( "b", jo["b"].name().c_str() );
	STRCMP_EQUAL( "c", jo["c"].name().c_str() );
	CHECK( jo.name().empty() );
}

TEST(JsonGroup, Empty)
{
	Json none;
	CHECK( none.empty() );
}

TEST(JsonGroup, ObjectHasName)
{
	Json jo;
	CHECK( Json::parse( "{\"a\":true,\"b\":1,\"c\":\"test\"}", jo ) );

	CHECK( jo.has( "a" ) );
	CHECK( jo.has( std::string( "b" ) ) );
	CHECK( jo.has( "c" ) );
	CHECK_FALSE( jo.has( "d" ) );
}

TEST(JsonGroup, ArrayModify)
{
	Json ja( Json::Array );
	CHECK_EQUAL( 0, ja.size() );
	CHECK( ja.empty() );

	ja.insert( 1 )
	  .insert( "test" );
	CHECK_EQUAL( 2, ja.size() );
	STRCMP_EQUAL( "test", ja.back().as_string().c_str() );

	ja.remove( 2 );
	CHECK_EQUAL( 2, ja.size() );
	ja.remove( 1 );
	CHECK_EQUAL( 1, ja.size() );
	CHECK_EQUAL( 1, ja.back().as_int() );
}

TEST(JsonGroup, ObjectModify)
{
	Json jo( Json::Object );
	CHECK_EQUAL( 0, jo.size() );
	CHECK( jo.empty() );

	jo.set( "a", true )
	  .set( std::string( "b" ), 1 )
	  .set( "c", "test" );
	CHECK_EQUAL( 3, jo.size() );
	CHECK( jo["a"].is_bool() );
	CHECK( jo["b"].is_number() );
	CHECK( jo["c"].is_string() );

	jo.remove( "b" );
	CHECK_EQUAL( 2, jo.size() );
	CHECK( jo["a"].is_bool() );
	CHECK( jo["b"].is_null() );
	CHECK( jo["c"].is_string() );
}

TEST(JsonGroup, Iterator)
{
	Json ja( { 1, 2 } );
	auto ai = ja.begin();
	CHECK( ai != ja.end() );
	CHECK_EQUAL( 1, ai->as_int() );
	ai++;
	CHECK( ai != ja.end() );
	CHECK_EQUAL( 2, ai->as_int() );
	ai++;
	CHECK( ai == ja.end() );

	Json jo;
	CHECK( Json::parse( "{\"a\":true,\"b\":1,\"c\":\"test\"}", jo ) );
	auto oi = jo.begin();
	CHECK( oi != jo.end() );
	STRCMP_EQUAL( "a", oi->name().c_str() );
	CHECK_EQUAL( true, oi->as_bool() );
	oi++;
	CHECK( oi != jo.end() );
	STRCMP_EQUAL( "b", oi->name().c_str() );
	CHECK_EQUAL( 1, oi->as_int() );
	oi++;
	CHECK( oi != jo.end() );
	STRCMP_EQUAL( "c", oi->name().c_str() );
	STRCMP_EQUAL( "test", oi->as_string().c_str() );
	oi++;
	CHECK( oi == jo.end() );
}

TEST(JsonGroup, EmptyStringTest)
{
	Json o;
	CHECK_FALSE( Json::parse( "", o ) );
	CHECK( o.empty() );
}

TEST(JsonGroup, LexemeTest)
{
	// "true" lexeme
	Json o;
	CHECK( Json::parse( "true", o ) );
	CHECK( o.is_bool() );

	// "false" lexeme
	CHECK( Json::parse( "false", o ) );
	CHECK( o.is_bool() );

	// "null" lexeme
	CHECK( Json::parse( "null", o ) );
	CHECK( o.is_null() );

	// Whitespaces
	CHECK( Json::parse( " \ttrue\t ", o ) );
	CHECK( o.is_bool() );

	// Bad lexeme
	CHECK_FALSE( Json::parse( "None", o ) );
	CHECK( o.is_null() );

	CHECK_FALSE( Json::parse( "bad", o ) );
	CHECK( o.is_null() );
}

TEST(JsonGroup, StringTest)
{
	Json o;
	CHECK( Json::parse( "\"\"", o ) );
	CHECK( Json::parse( "\" \"", o ) );
	CHECK( Json::parse( "\"test\"", o ) );
	CHECK( Json::parse( "\"test 123\"", o ) );
	CHECK( Json::parse( "\"A_1!!!\"", o ) );
	CHECK( Json::parse( "\"\\\\\"", o ) );
	CHECK( Json::parse( "\"\\/\"", o ) );
	CHECK( Json::parse( "\"\\b\"", o ) );
	CHECK( Json::parse( "\"\\f\"", o ) );
	CHECK( Json::parse( "\"\\n\"", o ) );
	CHECK( Json::parse( "\"\\r\"", o ) );
	CHECK( Json::parse( "\"\\t\"", o ) );
	CHECK( Json::parse( "\"\\u0123\"", o ) );
	CHECK_FALSE( Json::parse( "\"\\ubad\"", o ) );
}

TEST(JsonGroup, NumberTest)
{
	std::string valid_numbers[] = {
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "12", "123",
		"11", "22", "33", "44", "55", "66", "77", "88", "99",
		"-0", "-1", "-2", "-3", "-4", "-5", "-6", "-7", "-8", "-9", "-10",
		"-123", "-11", "-22", "-33", "-44", "-55", "-66", "-77", "-88", "-99",
		"0.0", "0.1", "0.2", "0.3", "0.4", "0.5", "0.6", "0.7", "0.8", "0.9", "1.2", "1.23",
		"1.1", "2.2", "3.3", "4.4", "5.5", "6.6", "7.7", "8.8", "9.9", "-0.0",
		"-1.1", "-2.2", "-3.3", "-4.4", "-5.5", "-6.6", "-7.7", "-8.8", "-9.9", "-10.01",
		"-12.3", "-11.1", "-22.2", "-33.3", "-44.4", "-55.5", "-66.6", "-77.7", "-88.8", "-99.9",
		"0e0", "1e1", "2e2", "3e3", "4e4", "5e5", "6e6", "7e7", "8e8", "9e9",
		"12e+12", "123e-123", "11e11", "22e22", "33e33", "44e44", "55e55", "66e66", "77e77",
		"88e88", "99e99", "-0e0", "-1e1", "-2e2", "-3e3", "-4e4", "-5e5", "-6e6", "-7e7",
		"-8e8", "-9e9", "-10e+2", "-123e-45", "-11e11", "-22e22", "-33e33", "-44e44", "-55e55",
		"-66e66", "-77e77", "-88e88", "-99e99", "0E0", "1E1", "2E2", "3E3", "4E4", "5E5",
		"6E6", "7E7", "8E8", "9E9", "12E+12", "123E-123", "11E11", "22E22", "33E33", "44E44",
		"55E55", "66E66", "77E77", "88E88", "99E99", "-0E0", "-1E1", "-2E2", "-3E3", "-4E4",
		"-5E5", "-6E6", "-7E7", "-8E8", "-9E9", "-10E+2", "-123E-45", "-11E11", "-22E22",
		"-33E33", "-44E44", "-55E55", "-66E66", "-77E77", "-88E88", "-99E99"
	};
	std::string invalid_numbers[] = { "-", "-e0", "-E0", "-E12", "e2E4", "e+", "E+" };

	Json o;
	for( const auto &i : valid_numbers )
	{
		CHECK( Json::parse( i, o ) );
	}
	for( const auto &i : invalid_numbers )
	{
		CHECK_FALSE( Json::parse( i, o ) );
	}
}

TEST(JsonGroup, ArrayTest)
{
	Json o;

	CHECK( Json::parse( "[]", o ) );
	CHECK( Json::parse( "[ ]", o ) );
	CHECK( Json::parse( "[\t]", o ) );
	CHECK( Json::parse( "[\r\n]", o ) );
	CHECK( Json::parse( "[\"test\"]", o ) );
	CHECK( Json::parse( "[\"test\",5,-123,true]", o ) );
	CHECK_FALSE( Json::parse( "[ \"test\" , 5 , -123 , true ", o ) );

	// Missing comma
	CHECK_FALSE( Json::parse( "[[ \"test\" , 5 true ]]", o ) );
}

TEST(JsonGroup, ObjectTest)
{
	Json o;

	CHECK( Json::parse( "{}", o ) );
	CHECK( Json::parse( "{ \t}", o ) );
	CHECK( Json::parse( " { \"key\" : \"value\" } ", o ) );
	CHECK( Json::parse( "{ \"key\" : \"value\", \"count\" : 5, \"price\" : 1.50 }", o ) );
	CHECK( Json::parse( "{\"key\":\"value\",\"count\":5,\"price\":1.50,\r\n\"valid\":true,\"promo_code\":null}", o ) );
	CHECK_FALSE( Json::parse( "{,}", o ) );
}

TEST(JsonGroup, CombinedTest)
{
	std::string str = R"_({
	"string": "test",
	"number": 123,
	"object": {
		"key": "value",
		"bool": true,
		"undef": null
	},
	"array": [
		123,
		false,
		"test"
	],
	"float": 1.5
	})_";

	Json o;
	CHECK( Json::parse( str, o ) );
}

TEST(JsonGroup, ParseCombinedTest)
{
	std::string str = R"_({
	"string": "test",
	"number": 123,
	"object": {
		"key": "value",
		"bool": true,
		"undef": null
	},
	"array": [
		0.1,
		false,
		"text"
	],
	"double": 1.5,
	"empty_array": [{}]
	})_";

	Json o;
	CHECK( Json::parse( str, o ) );

	STRCMP_EQUAL( "test", o["string"].as_string().c_str() );
	CHECK_EQUAL( 123, o["number"].as_int() );
	STRCMP_EQUAL( "value", o["object"]["key"].as_string().c_str() );
	CHECK( o["object"]["bool"].as_bool() );
	CHECK( o["object"]["undef"].is_null() );
	CHECK_EQUAL( 3, o["array"].size() );
	DOUBLES_EQUAL( 0.1, o["array"][0].as_float(), std::numeric_limits<float>::epsilon() );
	CHECK_FALSE( o["array"][1].as_bool() );
	STRCMP_EQUAL( "text", o["array"][2].as_string().c_str() );
	STRCMP_EQUAL( "text", o["array"].back().as_string().c_str() );
	DOUBLES_EQUAL( 1.5, o["double"].as_float(), std::numeric_limits<double>::epsilon() );
	CHECK_EQUAL( 1, o["empty_array"].size() );
	CHECK( Json::Type::Object == o["empty_array"][0].type() );

	STRCMP_EQUAL( "null", o["bad_key"].as_string().c_str() );
}

TEST(JsonGroup, MinimizeTest)
{
	std::string str = R"_({
	"string": "test",
	"number": 123,
	"object": {
		"key": "value",
		"bool": true,
		"undef": null
	},
	"array": [
		0.1,
		false,
		"text"
	],
	"double": 1.5,
	"empty_array": [{}]
	})_";
	auto v = Json::minimize( str );
	STRCMP_EQUAL( "{\"string\":\"test\",\"number\":123,\"object\":{\"key\":\"value\",\"bool\":true,\"undef\":null},\"array\":[0.1,false,\"text\"],\"double\":1.5,\"empty_array\":[{}]}", v.c_str() );
}

TEST(JsonGroup, ParseLexemeTest)
{
	Json o;
	CHECK( Json::parse( "true", o ) );
	CHECK( o.as_bool() );

	CHECK( Json::parse( "false", o ) );
	CHECK_FALSE( o.as_bool() );

	CHECK( Json::parse( "null", o ) );
	CHECK( o.empty() );
	CHECK( o.is_null() );
}

TEST(JsonGroup, ParseEmptyTest)
{
	Json o;
	CHECK_FALSE( Json::parse( "", o ) );
	CHECK( o.empty() );
	CHECK( o.is_null() );
}

TEST(JsonGroup, ParseNumberTest)
{
	Json o;
	CHECK( Json::parse( "0", o ) );
	CHECK_EQUAL( 0, o.as_int() );

	CHECK( Json::parse( "0.5", o ) );
	DOUBLES_EQUAL( 0.5, o.as_float(), std::numeric_limits<float>::epsilon() );

	CHECK( Json::parse( "-1.2", o ) );
	DOUBLES_EQUAL( -1.2, o.as_float(), std::numeric_limits<float>::epsilon() ); // We have float epsilon in this context
}

TEST(JsonGroup, ParseStringTest)
{
	Json o;
	CHECK( Json::parse( "\"str\"", o ) );
	STRCMP_EQUAL( "str", o.as_string().c_str() );
}

TEST(JsonGroup, ParseMiscTest)
{
	Json o;
	CHECK( Json::parse( "{\"a\": [], \"b\":{},\"c\":null}", o ) );
	CHECK( Json::parse( "{}", o ) );
	CHECK( Json::parse( "[]", o ) );
	CHECK( Json::parse( "{\"\":1}", o ) ); // For some reason cJson thinks this is a valid key
}

TEST(JsonGroup, InvalidInputTest)
{
	Json o;
	std::string scenarios[] = { "tru", "{", "}", "[", "]", "{\":1}" };
	for( const auto &i : scenarios )
	{
		CHECK_FALSE( Json::parse( i, o ) );
	}
}

TEST(JsonGroup, ToStringTest)
{
	Json o( Json::Type::Object );
	o.set( "string", "test" )
	 .set( "number", 123 )
	 .set( "object", Json( Json::Type::Object ) )
	 .set( "array", Json( Json::Type::Array ) )
	 .set( "float", 1.5 )
	 .set( "double", std::numeric_limits<double>::max() );

	o["object"].set( "key", "value" )
			   .set( "bool", true )
			   .set( "undef", Json( Json::Type::Null ) );
	o["array"].insert( 123 )
			  .insert( false )
			  .insert( "test" );

	auto s = o.build( true );
	STRCMP_CONTAINS( "\"array\":\t[", s.c_str() );
	STRCMP_CONTAINS( "\"float\":\t1.5", s.c_str() );
	STRCMP_CONTAINS( "\"double\":\t1.797", s.c_str() );
	STRCMP_CONTAINS( "\"number\":\t123", s.c_str() );
	STRCMP_CONTAINS( "\"object\":\t{", s.c_str() );
	STRCMP_CONTAINS( "\"bool\":\ttrue", s.c_str() );
	STRCMP_CONTAINS( "\"key\":\t\"value\"", s.c_str() );
	STRCMP_CONTAINS( "\"undef\":\tnull", s.c_str() );
	STRCMP_CONTAINS( "\"string\":\t\"test\"", s.c_str() );

	// overwrite
	o["object"].set( "key", "test value" );
	o["object"].set( "bool", false );
	o["array"][0] = 1234;
	o["array"][1] = true;

	s = o.build( true );
	STRCMP_CONTAINS( "\"key\":\t\"test value\"", s.c_str() );
	STRCMP_CONTAINS( "\"bool\":\tfalse", s.c_str() );
	CHECK_EQUAL( 1234, o["array"][0].as_int() );
	CHECK_EQUAL( true, o["array"][1].as_bool() );

	auto len = s.size();
	s = Json::minimize( s );
	CHECK( s.size() < len );
}

TEST(JsonGroup, Utf8Test)
{
	std::string utf8_str = u8"{\"цена\":\"10€\",\"количество\": 5}";

	Json o;
	CHECK( Json::parse( utf8_str, o ) );

	auto s = o.build( true );
	STRCMP_CONTAINS( "{", s.c_str() );
	STRCMP_CONTAINS( u8"\"цена\":\t\"10€\"", s.c_str() );
	STRCMP_CONTAINS( u8"\"количество\":\t5", s.c_str() );
	STRCMP_CONTAINS( "}", s.c_str() );

	s = o.build();
	STRCMP_CONTAINS( "{", s.c_str() );
	STRCMP_CONTAINS( u8"\"цена\":\"10€\"", s.c_str() );
	STRCMP_CONTAINS( u8"\"количество\":5", s.c_str() );
	STRCMP_CONTAINS( "}", s.c_str() );
}

TEST(JsonGroup, SpecialCharactersTest)
{
	Json v( " \" \\ \b \f \n \r \t " );
	auto dump = v.build();
	STRCMP_EQUAL( "\" \\\" \\\\ \\b \\f \\n \\r \\t \"", dump.c_str() );

	v = Json( Json::Type::Object );
	v.set( "string", " \" \\ \b \f \n \r \t " );

	dump = v.build();
	STRCMP_EQUAL( "{\"string\":\" \\\" \\\\ \\b \\f \\n \\r \\t \"}", dump.c_str() );

	Json res;
	CHECK( Json::parse( dump, res ) );
	STRCMP_EQUAL( " \" \\ \b \f \n \r \t ", res["string"].as_string().c_str() );
}
