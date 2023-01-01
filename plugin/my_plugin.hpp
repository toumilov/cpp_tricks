
#include <string>
#include "libplugin.hpp"

class MyAppPlugin : public plugin::Instance
{
public:
	MyAppPlugin();
	~MyAppPlugin();
	static const char* name();
	static const char* name_space();
	static plugin::Version version();
	virtual plugin::Identity id() const override;

	virtual std::string foo( int arg ) const;
};
