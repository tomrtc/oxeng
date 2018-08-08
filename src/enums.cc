#include <cstdlib>
#include <iostream>

#include <algorithm>
#include <chrono>
#include <functional>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <map>
#include <initializer_list>
#include <thread>
// C/Posix/linux headers.
#include <ftw.h>
#include <stdio.h>
#include <libgen.h> //basename(3) posix
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/stat.h>
#include "lowlevellib.hh"

//header only 3rd party libraries.

void show_usage(const char *progname);

std::string generated_path;
std::string input_file;
bool debug {false};



void
parser_managed_enum(std::string &managed_enum_source)
{
  std::string enum_name{"none"};

  std::smatch m;
  static const  std::regex& rgx_begin_name {R"raw(#ident \"managed_enum +(\w+))raw", std::regex::optimize};
  static const  std::regex& rgx_end_name {R"raw(#ident \"end_managed_enum +(\w+))raw", std::regex::optimize};
  static const  std::regex& rgx_declation_name {R"raw(enum[ \t]+(\w+).*\n?\{)raw", std::regex::optimize};


  if (std::regex_search(managed_enum_source, m, std::regex(rgx_begin_name)))
    {

      enum_name = m[1];

      if (std::regex_search(managed_enum_source, m, std::regex(rgx_end_name)))
	{
	  if (m[1] != enum_name)
	    {
	      std::cerr << "ERROR : incoherent managed_enum/end_managed_enum directives for "
			<< enum_name << std::endl;
	      std::exit(-1);
	    }
	  if (std::regex_search(managed_enum_source, m, std::regex(rgx_declation_name)))
	    {

	      if (m[1] != enum_name)
		{
		  std::cerr << "ERROR : incoherent managed_enum and actual enum declaration for "
			    << enum_name << " vs " << m[1] << std::endl;
		  std::exit(-1);
		}
	      std::cout << "Valid enum " << enum_name << std::endl;

	    }
	  else
	    {
	      std::cerr << "ERROR :  Incorrect enum declaration for " << enum_name
			<< " MUST be : enum " << enum_name << " { ..." << std::endl;
	      std::exit(-1);
	    }
	}
      else
	{
	  std::cerr << "ERROR : cannot find end_managed_enum directive for " << enum_name << std::endl;
	  std::exit(-1);
	}
    }
  else
    {
      std::cerr << "ERROR : Unrecognized managed_enum in code block :" << managed_enum_source << std::endl;
      std::exit(-1);
    }
}
std::string get_include_filename(const std::string t_prefix, const std::string &name, const std::string t_suffix)
{
  std::string tmp {t_prefix};
  tmp = tmp + name + t_suffix;
  return tmp;
}

std::string header_helper(std::string name)
{
  std::string cpp_guard {name};
  size_t npos = cpp_guard.find_last_of('.');
  cpp_guard[npos] = '_';
  std::transform(cpp_guard.begin(), cpp_guard.end(), cpp_guard.begin(), ::toupper);
  cpp_guard = "#ifndef " + cpp_guard + "\n#define " + cpp_guard + "\n/*Generated file do not modify!.*/\n\n";
  return cpp_guard;
}

std::vector<std::string>
split_pseudo_enum(const std::string& input,int submatch ) {
  static const std::regex& re{R"raw(#define[ \t]+(\w+).*\n)raw", std::regex::optimize};

  std::sregex_token_iterator
    first{input.begin(), input.end(), re, submatch},
    last;
    return {first, last};
}
void
parser_pseudo_enum(std::string &managed_enum_source)
{
  std::string pseudo_enum_name{"none"};

  std::smatch m;
  static const  std::regex& rgx_begin_name {R"raw(#ident \"pseudo_enum +(\w+))raw", std::regex::optimize};
  static const  std::regex& rgx_end_name {R"raw(#ident \"end_pseudo_enum +(\w+))raw", std::regex::optimize};

  if (std::regex_search(managed_enum_source, m, std::regex(rgx_begin_name)))
    {
      pseudo_enum_name = m[1];

      if (std::regex_search(managed_enum_source, m, std::regex(rgx_end_name)))
	{
	  if (m[1] != pseudo_enum_name)
	    {
	      std::cerr << "ERROR : incoherent pseudo_enum/end_pseudo_enum directives for "
			<< pseudo_enum_name << std::endl;
	      std::exit(-1);
	    }
	  std::cout << "Valid pseudo_enum " << pseudo_enum_name << std::endl;
	  std::string   gen_file_name {get_include_filename("gen_",pseudo_enum_name, "_str.hpp")};
	  std::ofstream gen_file(gen_file_name);
	  gen_file << header_helper(gen_file_name);
	  gen_file << R"raw(

const char*
get_enum_name(int value, char* pdefault)
{
switch(value) {

)raw";
	  const std::vector<std::string> pseudo_values { split_pseudo_enum(managed_enum_source,1)};
	  for (const auto& value: pseudo_values)
	    gen_file  << "case " << value  << " :\n\treturn \"" << value << "\";\n\tbreak;"<< std::endl;
	  gen_file << R"raw(
  default:
  return pdefault;
  }
}
#endif
)raw";
	  gen_file.close();
	}
      else
	{
	  std::cerr << "ERROR : cannot find end_pseudo_enum directive for " << pseudo_enum_name << std::endl;
	  std::exit(-1);
	}
    }
  else
    {
      std::cerr << "ERROR : Unrecognized pseudo_enum in code block :" << managed_enum_source << std::endl;
      std::exit(-1);
    }
}


bool
parser(const std::string &t_file)
{

  std::string file_content {get_file_contents(t_file)};
  static const  std::regex& rgx {R"raw(/\*([^%*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/)raw", std::regex::optimize};
  static const  std::regex& rgxcpp {R"raw(//.*)raw", std::regex::optimize};
  static const  std::regex& rgxblank {R"raw(\n[\t \n]+)raw", std::regex::optimize};
  static const std::string &delimiter_begin {"\n#ident \"managed_enum"};
  static const std::string &delimiter_end {"\n#ident \"end_managed_enum"};
  static const std::string &pseudo_begin {"\n#ident \"pseudo_enum"};
  static const std::string &pseudo_end {"\n#ident \"end_pseudo_enum"};
  //static const std::string delimiter_end   {"\n#ident "};


  for(size_t p=0, q=0; p!=file_content.npos; p=q) {
    std::string managed_enum_source {""};
    size_t position_begin_managed_enum  = file_content.find(delimiter_begin, p);

    if (position_begin_managed_enum  == std::string::npos)
      break;

    size_t position_end_managed_enum = file_content.find(delimiter_end, position_begin_managed_enum);
    if (position_end_managed_enum  == std::string::npos)
      return false;
    q  = file_content.find_first_of("\n", position_end_managed_enum+delimiter_end.size());

    managed_enum_source = file_content.substr(position_begin_managed_enum + 1, q -position_begin_managed_enum -1 );
    managed_enum_source = std::regex_replace(managed_enum_source, rgx, "");
    managed_enum_source = std::regex_replace(managed_enum_source, rgxcpp, "");
    managed_enum_source = std::regex_replace(managed_enum_source, rgxblank, "\n");
    // std::cout << "#{"<< managed_enum_source  << "}#\n"<< std::endl;
    parser_managed_enum(managed_enum_source);
  }

  for(size_t p=0, q=0; p!=file_content.npos; p=q) {
    std::string managed_enum_source {""};
    size_t position_begin_managed_enum  = file_content.find(pseudo_begin, p);

    if (position_begin_managed_enum  == std::string::npos)
      return false;

    size_t position_end_managed_enum = file_content.find(pseudo_end, position_begin_managed_enum);
    if (position_end_managed_enum  == std::string::npos)
      return false;
    q  = file_content.find_first_of("\n", position_end_managed_enum + pseudo_end.size());

    managed_enum_source = file_content.substr(position_begin_managed_enum + 1, q -position_begin_managed_enum -1 );
    managed_enum_source = std::regex_replace(managed_enum_source, rgx, "");
    managed_enum_source = std::regex_replace(managed_enum_source, rgxcpp, "");
    managed_enum_source = std::regex_replace(managed_enum_source, rgxblank, "\n");
    //std::cout << "#{"<< managed_enum_source  << "}#\n"<< std::endl;
    parser_pseudo_enum(managed_enum_source);
  }

  return true;
}



int
main (int argc,char *argv[])
{
  char     optchar;

  // avoid useless compatibilty with C.
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);

  std::chrono::high_resolution_clock::time_point
    tstart = std::chrono::high_resolution_clock::now();

  if(argc == 1)
    {
      show_usage(argv[0]);
      exit(1);
    }

  try {
    while((optchar=getopt(argc,argv,"hvg:f:"))!=-1)
      {
	switch(optchar)
	  {
	  case 'h':/*option h show the help*/
	    show_usage(argv[0]);
	    break;


	  case 'g': /*-g generated path */
	    generated_path = std::string(optarg);
	    break;

	  case 'f':/*-f pre-processed file from gnu cpp*/
	    input_file = std::string(optarg);

	    break;

	  case 'v':/*display the version.*/
	    std::cout<<"The current version is 0.1."<<std::endl;
	    break;

	  case 'd': /* debug and fun!*/
	    debug = true;
	    break;

	  default:
	    show_usage(argv[0]);
	    break;
	  }
      }
  } catch (int e)
    {
      std::cerr  << " : " << strerror(errno) << std::endl;
      return e;
    }
  catch (std::exception& e)
    {
      std::cerr << "exception caught: " << std::endl;
    }

  try {

  } catch (std::string e)
    {
      std::cerr  << e << " : " << strerror(errno) << std::endl;
      return errno;
    }


  int hardwareThreads = std::thread::hardware_concurrency();// number of threads supported by the hardware

  parser(input_file);


  std::chrono::high_resolution_clock::time_point
    tstop = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double>
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tstop - tstart);
  std::cout << "\nCPUs " << sysconf(_SC_NPROCESSORS_ONLN) << " - " << hardwareThreads <<" Time elapsed: " << time_span.count() << " s" << std::endl;
  return 0;
}


void
show_usage(const char *s)
{
  std::cout<<"Usage:   "<< s <<" [-option] [argument]"<<std::endl;
  std::cout<<"option:  "<<"-h show help information"<<std::endl;

  std::cout<<"         "<<"-f input_file.ii -- the result of gnu cpp"<<std::endl;
  std::cout<<"         "<<"-g generated_path -- where to create generated files."<<std::endl;
  std::cout<<"         "<<"-v show version infomation"<<std::endl;
  std::cout<<"example: "<< s <<" -f aggregate_all.ii -g /tmp"<<std::endl;
}
