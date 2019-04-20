#ifndef OPT_PARSE_HEADER
#define OPT_PARSE_HEADER
namespace Opt_Parse {
	struct Option {
		bool has_arg;
		const char * name;
		static Option create(const char * name, bool has_arg=false);
	};
	struct Result {
		bool is_option;
		// Option
		Option option;
		char * arg;
		// Nonoption
		char * name;
		
		static Result create_option(Option option, char * string);
		static Result create_nonoption(char * string);
		char * debug_str();
		bool is(const char * s);
	};
	struct Parser {
		Option * options;
		size_t options_size;
		size_t argv_index;
		int argc;
		char ** argv;
		void init(Option * options, size_t options_size, int argc, char ** argv);
		bool exhausted();
		bool locate_option(char * arg, Option * out);
		Result next();
	};
};
#endif

#ifdef OPT_PARSE_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

using namespace Opt_Parse;

static bool string_match(const char * a, const char * b)
{
	return strcmp(a, b) == 0;
}

static bool starts_with(const char * a, const char * b)
{
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);
	for (int i = 0; i < b_len; i++) {
		if (i >= a_len) {
			return false;
		}
		if (a[i] != b[i]) {
			return false;
		}
	}
	return true;
}

static void fatal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("Command line error:\n");
	vfprintf(stderr, fmt, args);
	printf("\n");

	va_end(args);
	exit(1);
}

Option Option::create(const char * name, bool has_arg)
{
	return (Option) { has_arg, name };
}

Result Result::create_option(Option option, char * arg)
{
	Result result;
	result.is_option = true;
	result.option = option;
	result.arg = arg;
	return result;
}
Result Result::create_nonoption(char * name)
{
	Result result;
	result.is_option = false;
	result.name = name;
	return result;
}

char * Result::debug_str()
{
	char buf[1024];
	if (is_option) {
		if (option.has_arg) {
			sprintf(buf, "{ option: %s %s }", option.name, arg);
		} else {
			sprintf(buf, "{ option: %s }", option.name);
		}
	} else {
		sprintf(buf, "{ nonoption: %s }", name);
	}
	return strdup(buf);
}

bool Result::is(const char * s)
{
	if (is_option) {
		return string_match(option.name, s);
	} else {
		return string_match(name, s);
	}
}

void Parser::init(Option * options, size_t options_size, int argc, char ** argv)
{
	this->options = options;
	this->options_size = options_size;
	this->argc = argc;
	this->argv = argv;
	argv_index = 1;
}

bool Parser::exhausted()
{
	return argv_index >= argc;
}

bool Parser::locate_option(char * arg, Option * out)
{
	for (int i = 0; i < options_size; i++) {
		auto opt = options[i];
		if (string_match(arg, opt.name)) {
			*out = opt;
			return true;
		}
	}
	return false;
}

Result Parser::next()
{
	if (argv_index >= argc) {
		fatal("Tried to grab next command-line argument when iterator is exhausted");
	}
	char * flag = argv[argv_index++];
	if (starts_with(flag, "-")) {
		if (starts_with(flag, "--")) {
			flag = flag + 2;
		} else {
			flag = flag + 1;
		}
		Option option;
		if (!locate_option(flag, &option)) {
			fatal("No such option as '%s'", flag);
		}
		auto result = Result::create_option(option, flag);
		if (option.has_arg) {
			if (exhausted()) {
				fatal("Expected argument to '%s'", flag);
			}
			result.arg = argv[argv_index++];
		}
		return result;
	}
	// Non-option
	return Result::create_nonoption(flag);
}

#endif
