/**
 * @file
 * This file is part of SWE.
 *
 * @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 * Command line argument parser
 */

#ifndef TOOLS_ARGS_H
#define TOOLS_ARGS_H

#include <getopt.h>
#include <algorithm>
#include <map>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

namespace tools
{

/**
 * Parses command line arguments
 */
class Args
{
private:
	struct optionInfo {
		std::string longOption;		// We need a copy here to get the const char* correct
		/** Name of the value in the help command */
		std::string value;
		std::string description;
		bool required;
	};

	/**
	 * Convert a long option into an "argument" that is shown in the help message
	 */
	struct valueConvert {
		void operator()(char& c)
		{
			c = toupper(static_cast<unsigned char>(c));
			switch (c) {
			case '-':
				c = '_';
				break;
			}
		}
	};

	/** Program description (can be empty) */
	const std::string m_description;
	/** Automatically add help option */
	const bool m_addHelp;

	/** The command line arguments */
	std::vector<struct option> m_options;
	/**
	 * Additional information for the command line arguments
	 * required to generate help information
	 */
	std::vector<optionInfo> m_optionInfo;

	/** Maps from short option to index in m_options */
	std::map<char, size_t> m_short2option;

	/** Contains the arguments after parse was called */
	std::map<std::string, std::string> m_arguments;

public:
	enum Argument
	{
		Required = required_argument,
		No = no_argument,
		Optional = optional_argument
	};

	enum Result
	{
		Success = 0,
		Error,
		/** Help message printed */
		Help
	};

public:
	Args(const std::string &description = "", bool addHelp = true)
		: m_description(description),
		  m_addHelp(addHelp)
	{
	}

	void addOption(const std::string &longOption,
			char shortOption = 0,
			const std::string& description = "",
			Argument argument = Required,
			bool required = true)
	{
		std::string value;

		if (shortOption)
			m_short2option[shortOption] = m_options.size();

		if (argument != No) {
			value = longOption;
			for_each(value.begin(), value.end(), valueConvert());
		}

		struct optionInfo i = {longOption, value, description, required};
		m_optionInfo.push_back(i);

		struct option o = {m_optionInfo.back().longOption.c_str(), argument, 0, shortOption};
		m_options.push_back(o);
	}

	/**
	 * @return True of options are successfully parsed, false otherwise
	 */
	Result parse(int argc, char* const* argv, bool printHelp = true)
	{
		if (m_addHelp)
			addOption("help", 'h', "Show this help message", No, false);

		std::ostringstream shortOptions;
		for (std::vector<struct option>::const_iterator i = m_options.begin();
			i != m_options.end(); i++) {
			if (i->val != 0) {
				shortOptions << static_cast<char>(i->val);
				switch (i->has_arg)
				{
				case required_argument:
					shortOptions << ':';
					break;
				case optional_argument:
					shortOptions << "::";
					break;
				}
			}
		}

		// Add null option
		struct option o = {0, 0, 0, 0};
		m_options.push_back(o);

		// Update const char* in m_options
		for (size_t i = 0; i < m_optionInfo.size(); i++)
			m_options[i].name = m_optionInfo[i].longOption.c_str();

		while (true) {
			int optionIndex = 0;

			int c = getopt_long(argc, argv, shortOptions.str().c_str(),
				&m_options[0], &optionIndex);

			if (c < 0)
				break;

			switch (c) {
			case '?':
				if (printHelp)
					helpMessage(argv[0], std::cerr);
				return Error;
			case 0:
				// Nothing to do
				break;
			default:
				optionIndex = m_short2option.at(c);
			}

			if (optarg == 0L)
				m_arguments[m_options[optionIndex].name] = "";
			else
				m_arguments[m_options[optionIndex].name] = optarg;
		}

		if (m_addHelp && isSet("help")) {
			if (printHelp)
				helpMessage(argv[0]);
			return Help;
		}

		for (std::vector<optionInfo>::const_iterator i = m_optionInfo.begin();
			i != m_optionInfo.end(); i++) {
			if (i->required && !isSet(i->longOption)) {
				if (printHelp) {
					std::cerr << argv[0] << ": option --" << i->longOption << " is required" << std::endl;
					helpMessage(argv[0], std::cerr);
				}
				return Error;
			}
		}

		return Success;
	}

	bool isSet(const std::string &option)
	{
		return m_arguments.find(option) != m_arguments.end();
	}

	template<typename T>
	T getArgument(const std::string &option)
	{
		std::istringstream ss(m_arguments.at(option));

		T result;
		ss >> result;

		return result;
	}

	template<typename T>
	T getArgument(const std::string &option, T defaultArgument)
	{
		if (!isSet(option))
			return defaultArgument;

		return getArgument<T>(option);
	}

	void helpMessage(const char* prog, std::ostream &out = std::cout)
	{
		// First line with all short options
		out << "Usage: " << prog;
		for (size_t i = 0; i < m_options.size()-1; i++) {
			out << ' ';

			if (!m_optionInfo[i].required)
				out << '[';

			if (m_options[i].val != 0)
				out << '-' << static_cast<char>(m_options[i].val);
			else
				out << "--" << m_options[i].name;

			argumentInfo(i, out);

			if (!m_optionInfo[i].required)
				out << ']';
		}
		out << std::endl;

		// General program description
		if (!m_description.empty())
			out << std::endl << m_description << std::endl;

		// Optional arguments
		out << std::endl << "optional arguments:" << std::endl;
		for (size_t i = 0; i < m_options.size()-1; i++) {
			out << "  ";

			// Number of characters used for the option
			size_t length = 2;

			if (m_options[i].val != 0) {
				out << '-' << static_cast<char>(m_options[i].val);
				out << ", ";
				length += 4;
			}

			out << "--" << m_options[i].name;
			length += m_optionInfo[i].longOption.size() + 2;
			length += argumentInfo(i, out);

			if (length >= 30) {
				out << std::endl;
				out << std::setw(30) << ' ';
			} else
				out << std::setw(30-length) << ' ';

			out << m_optionInfo[i].description << std::endl;
		}
	}

private:
	/**
	 * Writes the argument information to out
	 *
	 * @param i The index of the option for which the argument should be generated
	 * @return The number if characters written
	 */
	size_t argumentInfo(size_t i, std::ostream &out)
	{
		switch (m_options[i].has_arg) {
		case required_argument:
			out << ' ' << m_optionInfo[i].value;
			return m_optionInfo[i].value.size() + 1;
		case optional_argument:
			out << " [" << m_optionInfo[i].value << ']';
			return m_optionInfo[i].value.size() + 3;
		}

		return 0;
	}
};

template<> inline
std::string tools::Args::getArgument(const std::string &option)
{
	return m_arguments.at(option);
}

}

#endif // TOOLS_ARGS_H
