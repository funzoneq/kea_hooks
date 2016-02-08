#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include "library_common.h"
#include <string>
#include <sstream>
#include <iostream>
#include <regex>

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
extern "C" {

// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle) {
    try {
        Pkt4Ptr response4_ptr;
        handle.getArgument("response4", response4_ptr);

	string res;

	// Get main options
	OptionPtr option82_ptr = response4_ptr->getOption(82);
	OptionPtr option43_ptr = response4_ptr->getOption(43);

	// Get sub options
	OptionPtr option82_1_ptr = option82_ptr->getOption(1);
	OptionPtr option82_2_ptr = option82_ptr->getOption(2);

	OptionPtr option43_1_ptr = option43_ptr->getOption(1);

	// Decode options to strings
	string option82_1_data = option82_1_ptr->toText();
	string option82_2_data = option82_2_ptr->toText();
	string option43_1_data = option43_1_ptr->toText();

	// The string contains 19 bytes of header-data...
	option82_2_data = option82_2_data.substr(19,  string::npos);


	// Decode :-separated string of ascii-codes
        stringstream ss(option82_2_data);
        string token;
        while(getline(ss, token, ':')) 
        {
                res += strtoul(token.c_str(), NULL, 16);
	}

	// Sanitize string before using it
        regex sanitize ("[^A-z0-9-]"); 
	res = regex_replace (res, sanitize, "_");

	// Replace "variable" in original packet data
	// 
	// "option_data": { "data": "foo_bar_%OPTION82_1%_baz"; }
	//
	regex opt82_1 ("@OPTION82_1@");
	regex opt82_2 ("@OPTION82_2@");

	option43_1_data = regex_replace (option43_1_data, opt82_1, res);
	option43_1_data = regex_replace (option43_1_data, opt82_2, res);

	// TODO: How to actually insert the new string into the packet?
	//
	// option43_ptr->setData(option43_1_data);

        // Write the information to the log file.
        interesting << option82_2_data << " " << res << " " << option43_1_data << "\n";
        // ... and to guard against a crash, we'll flush the output stream.
        flush(interesting);
    } catch (const NoSuchCalloutContext&) {
        // No such element in the per-request context with the name "hwaddr".
        // This means that the request was not an interesting, so do nothing
        // and dismiss the exception.
        interesting << "Que Pasa\n";
        flush(interesting);
    }
    return (0);
}


}