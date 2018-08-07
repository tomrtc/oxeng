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


bool
parser(const std::string &t_file)
{

  std::string file_content {get_file_contents(t_file)};
  static const  std::regex& rgx {R"raw(/\*([^%*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/)raw", std::regex::optimize};
  static const  std::regex& rgxcpp {R"raw(//.*)raw", std::regex::optimize};
  static const  std::regex& rgxblank {R"raw(\n[\t \n]+)raw", std::regex::optimize};

  static const std::string &delimiter_begin {"\n#ident \"managed_enum"};

  static const std::string &delimiter_end {"\n#ident \"end_managed_enum"};
  //static const std::string delimiter_end   {"\n#ident "};

  for(size_t p=0, q=0; p!=file_content.npos; p=q) {
    std::string managed_enum_source {""};
    size_t position_begin_managed_enum  = file_content.find(delimiter_begin, p);

    if (position_begin_managed_enum  == std::string::npos)
      return false;

    size_t position_end_managed_enum = file_content.find(delimiter_end, position_begin_managed_enum);
    if (position_end_managed_enum  == std::string::npos)
      return false;
    q  = file_content.find_first_of("\n", position_end_managed_enum+delimiter_end.size());

    managed_enum_source = file_content.substr(position_begin_managed_enum + 1, q -position_begin_managed_enum -1 );
    managed_enum_source = std::regex_replace(managed_enum_source, rgx, "");
    managed_enum_source = std::regex_replace(managed_enum_source, rgxcpp, "");
    managed_enum_source = std::regex_replace(managed_enum_source, rgxblank, "\n");



    std::cout << "#{"<< managed_enum_source  << "}#\n"<< std::endl;
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
