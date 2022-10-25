#pragma once

#include "cjson/cJSON.h"
#include <stdint.h>
#include <cmath>
#include <limits>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <functional>


class Json
{
	struct Data
	{
		cJSON *data;
		bool own;
		Data( cJSON *obj, bool own ) :
			data( obj ),
			own( own )
		{}
		~Data()
		{
			if ( own )
			{
				cJSON_Delete( data );
			}
		}

		inline cJSON* operator->()
		{
			return data;
		}
	private:
		Data( const Data& ) = delete;
		Data( const Data&& ) = delete;
		Data& operator=( const Data& ) = delete;
		Data& operator=( const Data&& ) = delete;
	};

	typedef std::shared_ptr<Data> DataPtr;
	typedef std::set<std::pair<Json, cJSON*> > ObjectSet;
	typedef std::shared_ptr<ObjectSet> ObjectSetPtr;
	DataPtr data_;
	ObjectSetPtr refs_; // Array/object sub-item references

public:

	enum Type {
		Null,
		Bool,
		Number,
		String,
		Array,
		Object
	};

	struct Iterator
	{
		Iterator& operator++()
		{
			if ( i_ )
			{
				i_ = i_->next;
			}
			return *this;
		}
		Iterator& operator++( int )
		{
			if ( i_ )
			{
				i_ = i_->next;
			}
			return *this;
		}
		bool operator==( const Iterator &rhs ) const
		{
			return i_ == rhs.i_;
		}
		bool operator!=( const Iterator &rhs ) const
		{
			return i_ != rhs.i_;
		}
		const Json operator*() const
		{
			return Json( i_, false );
		}
		const Json operator->() const
		{
			return Json( i_, false );
		}

	private:
		friend class Json;
		cJSON *i_;

		Iterator( cJSON *c ) :
			i_( c )
		{}
	};

	/**
	 * Parse text into object
	 * @param json Json text
	 * @param e result status
	 * @return Json string
	 */
	static bool parse( const char *json, Json &jo )
	{
		cJSON* cjson = cJSON_Parse( json );
		if ( cjson )
		{
			jo = Json( cjson, true );
			return true;
		}
		jo.clean();
		return false;
	}

	static inline bool parse( const std::string &json, Json &jo )
	{
		return parse( json.c_str(), jo );
	}

	/**
	 * Make Json more compact
	 * @param json Json text
	 * @return compact Json string
	 */
	static std::string minimize( const std::string &json )
	{
		std::string tmp( json );
		cJSON_Minify( &tmp[0] );
		return std::string( tmp.c_str() );
	}

	Json() :
		data_( new Data( cJSON_CreateNull(), true ) ),
		refs_( new ObjectSet )
	{}

	Json( cJSON *obj, bool own ) :
		data_( new Data( obj, own ) ),
		refs_( new ObjectSet )
	{}

	explicit Json( const Type t ) : Json()
	{
		switch( t )
		{
			case Bool:
				data_.reset( new Data( cJSON_CreateFalse(), true ) );
				break;
			case Number:
				data_.reset( new Data( cJSON_CreateNumber( 0.0 ), true ) );
				break;
			case String:
				data_.reset( new Data( cJSON_CreateString( "" ), true ) );
				break;
			case Array:
				data_.reset( new Data( cJSON_CreateArray(), true ) );
				break;
			case Object:
				data_.reset( new Data( cJSON_CreateObject(), true ) );
				break;
			default:
				break;
		}
	}

	explicit Json( bool value ) :
		data_( new Data( value ? cJSON_CreateTrue() : cJSON_CreateFalse(), true ) )
	{}

	explicit Json( double value ) :
		data_( new Data( cJSON_CreateNumber( value ), true ) )
	{}

	explicit Json( int value ) :
		data_( new Data( cJSON_CreateNumber( static_cast<double>( value ) ), true ) )
	{}

	explicit Json( unsigned value ) :
		data_( new Data( cJSON_CreateNumber( static_cast<double>( value ) ), true ) )
	{}

	explicit Json( const char *value ) :
		data_( new Data( cJSON_CreateString( value ), true ) )
	{}

	explicit Json( const std::string &value ) :
		data_( new Data( cJSON_CreateString( value.c_str() ), true ) )
	{}

	template <typename T, template<typename X, typename A> class ContT=std::vector>
	explicit Json( const ContT<T, std::allocator<T> >& elems ) :
		data_( new Data( cJSON_CreateArray(), true ) ),
		refs_( new ObjectSet )
	{
		for( typename ContT<T, std::allocator<T> >::const_iterator it = elems.begin();
			 it != elems.end(); it++ )
		{
			insert( *it );
		}
	}

	template <typename T>
	Json( const std::initializer_list<T> &elems ) :
		data_( new Data( cJSON_CreateArray(), true ) ),
		refs_( new ObjectSet )
	{
		for( auto &it : elems )
		{
			insert( it );
		}
	}

	template <typename T, template<typename X> class ContT>
	explicit Json( const ContT<T> &elems ) :
		data_( new Data( cJSON_CreateArray(), true ) ),
		refs_( new ObjectSet )
	{
		for( typename ContT<T>::const_iterator it = elems.begin(); it != elems.end(); it++ )
		{
			insert( *it );
		}
	}

	Json( const Json &rhs ) :
		data_( rhs.data_ ),
		refs_( rhs.refs_ )
	{
	}

	Json( Json &&rhs ) :
		data_( std::move( rhs.data_ ) ),
		refs_( std::move( rhs.refs_ ) )
	{}

	~Json()
	{}

	void swap( Json &rhs )
	{
		data_.swap( rhs.data_ );
		refs_.swap( rhs.refs_ );
	}

	void swap( Json &&rhs )
	{
		data_.swap( rhs.data_ );
		refs_.swap( rhs.refs_ );
	}

	bool operator<( const Json &rhs ) const
	{
		return data_->data < rhs.data_->data;
	}

	const Json* operator->() const
	{
		return this;
	}

	// Assignment
	inline Json& operator=( const Json &rhs )
	{
		if ( &rhs != this )
		{
			data_ = rhs.data_;
			refs_ = rhs.refs_;
		}
		return *this;
	}

	Json& operator=( const char *value )
	{
		if ( !is_string() )
		{
			swap( Json( String ) );
		}
		cJSON_SetValuestring( data_->data, value );
		return *this;
	}

	Json& operator=( const std::string &value )
	{
		operator=( value.c_str() );
		return *this;
	}

	Json& operator=( bool value )
	{
		if ( !is_bool() )
		{
			swap( Json( Bool ) );
		}
		data_->data->type = ( data_->data->type &
				( ~( cJSON_False | cJSON_True ) ) ) |
				( value ? cJSON_True : cJSON_False );
		return *this;
	}

	template<typename T,
			 typename std::enable_if<std::is_same<T, int>::value      ||
									 std::is_same<T, int32_t>::value  ||
									 std::is_same<T, int64_t>::value  ||
									 std::is_same<T, uint32_t>::value ||
									 std::is_same<T, uint64_t>::value ||
									 std::is_same<T, float>::value    ||
									 std::is_same<T, double>::value>::type* = nullptr>
	Json& operator=( const T &value )
	{
		if ( !is_number() )
		{
			swap( Json( Number ) );
		}
		cJSON_SetNumberHelper( data_->data, static_cast<double>( value ) );
		return *this;
	}

	// Comparison
	bool operator==( bool value ) const
	{
		if ( type() != Bool )
		{
			return false;
		}
		return ( cJSON_IsTrue( data_->data ) && value ) ||
				( cJSON_IsFalse( data_->data ) && !value );
	}

	bool operator==( const char *value ) const
	{
		if ( type() != String )
		{
			return false;
		}
		return std::string( data_->data->valuestring ) == value;
	}

	inline bool operator==( const std::string &value ) const
	{
		return operator==( value.c_str() );
	}

	template<typename T,
			 typename std::enable_if<std::is_same<T, int>::value      ||
									 std::is_same<T, int32_t>::value  ||
									 std::is_same<T, int64_t>::value  ||
									 std::is_same<T, uint32_t>::value ||
									 std::is_same<T, uint64_t>::value ||
									 std::is_same<T, float>::value    ||
									 std::is_same<T, double>::value>::type* = nullptr>
	bool operator==( const T value )
	{
		if ( type() != Number )
		{
			return false;
		}
		return fabs( data_->data->valuedouble - static_cast<double>( value ) ) < std::numeric_limits<double>::epsilon();
	}

	bool operator==( const Json &value ) const
	{
		return data_->data && value.data_->data && cJSON_Compare( data_->data, value.data_->data, 1 );
	}

	template <typename T>
	bool operator!=( const T &value ) const
	{
		return !( *this == value );
	}

	/**
	 * @brief Operator bool. Checks if value is empty.
	 */
	operator bool() const
	{
		return type() == Null;
	}

	void clean()
	{
		Json jo;
		swap( jo );
	}

	/**
	 * @brief Returns an element by specified index or Null object if no such element.
	 * @param index Element index.
	 * @return Object reference.
	 */
	inline Json operator[]( int index )
	{
		return at( index );
	}

	/**
	 * @brief Returns an element by key or Null object if no such element.
	 * @param key Element key.
	 * @return Object reference.
	 */
	inline Json operator[]( const std::string &key )
	{
		return at( key );
	}

	inline const Json operator[]( const std::string &key ) const
	{
		return at( key );
	}

	inline Json operator[]( const char *key )
	{
		return at( key );
	}

	inline const Json operator[]( const char *key ) const
	{
		return at( key );
	}

	Iterator begin() const
	{
		if ( !data_ || !data_->data )
		{
			return Iterator( nullptr );
		}
		return Iterator( data_->data->child );
	}

	const Iterator end() const
	{
		return Iterator( nullptr );
	}

	/**
	 * Returns object type
	 */
	inline Type type() const
	{
		switch( data_->data->type & 0xff )
		{
			case cJSON_False:
			case cJSON_True:
				return Bool;
			case cJSON_Number:
				return Number;
			case cJSON_String:
				return String;
			case cJSON_Array:
				return Array;
			case cJSON_Object:
				return Object;
			default:
				return Null;
		}
	}

	inline bool is( Type t ) const
	{
		return type() == t;
	}

	/*
	 * Returns object's child name
	 */
	std::string name() const
	{
		if ( data_->data->string )
		{
			return std::string( data_->data->string );
		}
		return std::string();
	}

	/**
	 * Returns true if object is none
	 */
	inline bool empty() const
	{
		switch( type() )
		{
			case Null:
				return true;
			case Array:
			case Object:
				return data_->data->next == nullptr && data_->data->prev == nullptr;;
			default:
				break;
		}
		return false;
	}

	bool has( const char *key ) const
	{
		if ( type() != Object )
		{
			return false;
		}
		return cJSON_HasObjectItem( data_->data, key );
	}

	inline bool has( const std::string &key ) const
	{
		return has( key.c_str() );
	}

	/**
	 * at Returns an element by specified index or Null object if no such element.
	 * @param index Element index.
	 * @return Object reference.
	 */
	Json at( int index )
	{
		if ( is_array() )
		{
			cJSON* item = cJSON_GetArrayItem( data_->data, index );
			if ( item )
			{
				auto o = Json( item, false );
				for( ObjectSet::iterator it = refs_->begin(); it != refs_->end(); it++ )
				{
					if ( it->second == item )
					{
						o.refs_ = it->first.refs_;
						break;
					}
				}
				return o;
			}
		}
		return Json();
	}

	/**
	 * at Returns an element by key or Null object if no such element.
	 * @param key Element key.
	 * @return Object reference.
	 */
	inline Json at( const std::string &key )
	{
		return at( key.c_str() );
	}

	inline const Json at( const std::string &key ) const
	{
		return at( key.c_str() );
	}

	Json at( const char *key )
	{
		if ( type() != Object )
		{
			return Json();
		}
		cJSON* item = cJSON_GetObjectItemCaseSensitive( data_->data, key );
		if ( item )
		{
			auto o = Json( item, false );
			for( ObjectSet::iterator it = refs_->begin(); it != refs_->end(); it++ )
			{
				if ( it->second == item )
				{
					o.refs_ = it->first.refs_;
					break;
				}
			}
			return o;
		}
		return Json();
	}

	const Json at( const char *key ) const
	{
		if ( type() != Object )
		{
			return Json();
		}
		cJSON* item = cJSON_GetObjectItemCaseSensitive( data_->data, key );
		if ( item )
		{
			auto o = Json( item, false );
			for( ObjectSet::iterator it = refs_->begin(); it != refs_->end(); it++ )
			{
				if ( it->second == item )
				{
					o.refs_ = it->first.refs_;
					break;
				}
			}
			return o;
		}
		return Json();
	}

	/**
	 * Array items count
	 * @return elements number
	 */
	int size() const
	{
		if ( is_array() )
		{
			return cJSON_GetArraySize( data_->data );
		}
		else if ( is_object() && refs_ )
		{
			return refs_->size();
		}
		return 0;
	}

	/**
	 * Last array element
	 */
	Json back() const
	{
		int index = size();
		if ( is_array() && index > 0 )
		{
			index--;
			cJSON* item = cJSON_GetArrayItem( data_->data, index );
			if ( item )
			{
				return Json( item, false );
			}
		}
		return Json();
	}

	/**
	 * insert Adds a new element into array.
	 * @param v Object.
	 * @return Object reference (this).
	 */
	template <typename T>
	Json& insert( const T &value )
	{
		if ( is_array() )
		{
			Json o( value );
			int i = cJSON_GetArraySize( data_->data );
			cJSON_AddItemReferenceToArray( data_->data, o.data_->data );
			cJSON *ref = cJSON_GetArrayItem( data_->data, i );
			refs_->insert( std::pair<Json, cJSON*>( std::move( o ), ref ) );
		}
		return *this;
	}

	/**
	 * Adds a new element into object.
	 * @param key Object key.
	 * @param v Object itself.
	 * @return Object reference (this).
	 */
	template <typename T>
	Json& set( const char *key, const T &value )
	{
		if ( is_object() )
		{
			if ( has( key ) )
			{
				remove( key );
			}
			Json o( value );
			cJSON_AddItemReferenceToObject( data_->data, key, o.data_->data );
			cJSON *ref = cJSON_GetObjectItemCaseSensitive( data_->data, key );
			refs_->insert( std::pair<Json, cJSON*>( std::move( o ), ref ) );
		}
		return *this;
	}

	template <typename T>
	inline Json& set( const std::string &name, const T &value )
	{
		return set( name.c_str(), value );
	}

	/**
	 * Removes element from array by index.
	 * @param index Element index.
	 * @return Object reference (this).
	 */
	Json& remove( int index )
	{
		if ( is_array() )
		{
			cJSON* detached = cJSON_DetachItemFromArray( data_->data, index );
			if (detached)
			{
				for( ObjectSet::iterator it = refs_->begin(); it != refs_->end(); it++ )
				{
					if ( it->first.data_->data == detached )
					{
						refs_->erase( it );
						break;
					}
				}
				cJSON_Delete( detached );
			}
		}
		return *this;
	}

	/**
	 * Removes element from object
	 * @param index Element index
	 * @return Object reference (this)
	 */
	Json& remove( const char *name )
	{
		if ( is_object() )
		{
			cJSON* detached = cJSON_DetachItemFromObject( data_->data, name );
			if ( detached )
			{
				for( ObjectSet::iterator it = refs_->begin(); it != refs_->end(); it++ )
				{
					if ( it->second == detached )
					{
						refs_->erase( it );
						break;
					}
				}
				cJSON_Delete( detached );
			}
		}
		return *this;
	}

	Json& remove( const std::string &name )
	{
		return remove( name.c_str() );
	}

	inline bool is_null() const
	{
		return type() == Null;
	}

	inline bool is_bool() const
	{
		return type() == Bool;
	}

	inline bool is_number() const
	{
		return type() == Number;
	}

	inline bool is_string() const
	{
		return type() == String;
	}

	inline bool is_array() const
	{
		return type() == Array;
	}

	inline bool is_object() const
	{
		return type() == Object;
	}

	int as_int() const
	{
		if ( is_number() )
		{
			return data_->data->valueint;
		}
		return 0;
	}

	unsigned as_uint() const
	{
		if ( is_number() )
		{
			return (unsigned)data_->data->valueint;
		}
		return 0u;
	}

	int64_t as_int64() const
	{
		if ( is_number() )
		{
			return static_cast<int64_t>( data_->data->valuedouble );
		}
		return 0;
	}

	std::string as_string() const
	{
		switch( data_->data->type & 0xff )
		{
			case cJSON_False:
				return "false";
			case cJSON_True:
				return "true";
			case cJSON_Number:
				return std::to_string( as_int() );
			case cJSON_String:
				return data_->data->valuestring;
			case cJSON_Array:
			case cJSON_Object:
				return "";
			default:
				return "null";
		}
		if ( is_string() )
		{
			return data_->data->valuestring;
		}
		return std::string();
	}

	double as_float() const
	{
		if ( is_number() )
		{
			return data_->data->valuedouble;
		}
		return 0.0;
	}

	bool as_bool() const
	{
		if ( ( data_->data->type & 0xff ) == cJSON_True )
		{
			return true;
		}
		return false;
	}

	inline std::vector<Json> as_array() const
	{
		std::vector<Json> retval;
		if ( is_array() )
		{
			for( int i = 0; i < cJSON_GetArraySize( data_->data ); i++ )
			{
				retval.push_back( Json( cJSON_GetArrayItem( data_->data, i ), false ) );
			}
		}
		return retval;
	}

	/**
	 * Dump object into Json string
	 * @param formatted pretty print
	 * @return Json string
	 */
	std::string build( bool formatted = false ) const
	{
		char *json = formatted ? cJSON_Print( data_->data ) : cJSON_PrintUnformatted( data_->data );
		std::string retval( json );
		free( json );
		return retval;
	}
};

