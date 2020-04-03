/*++
------------------------------------------------------------------------

                    converters.C

     Identification

           Project         :

     Reference Documents   :

     Author                : Remy Tomasetto
     Date                  : 09/03/2020

     Functionality         : Strings generation tool

     History
------------------------------------------------------------------------
++*/

#include "oxeng.hh"

#include <cstdlib>

#include <vector>
#include <string>
#include <unordered_map>
#include <regex>
//header only 3rd party libraries.
#include <iostream>     // std::cout, std::ostream, std::ios
#include <fstream>      // std::filebuf
#include <sstream>
#include <iomanip>      // std::setw

// C/Posix/linux headers.
// no more necessary #define _XOPEN_SOURCE 500
#include <stdio.h>
#include <libgen.h> //basename(3) posix
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/stat.h>


 static const std::unordered_map<std::string, const char *>  map_utf7_x {
   {"\\x01", u8""},{"\\x02", u8""},{"\\x03", u8"φ"},{"\\x04", u8"°"}
 };

static const std::unordered_map<std::string, const char *>  map_utf7_a {
  {"\\a'Y", u8"Ý"},{"\\a'y", u8"ý"}, {"\\a'A", u8"Á"},{"\\a'I", u8"Í"},{"\\a'O", u8"Ó"},{"\\a`A", u8"À"},{"\\a`E", u8"È"},{"\\a`I", u8"Ì"},
  {"\\a`O", u8"Ò"},{"\\a^A", u8"Â"},{"\\a^E", u8"Ê"},{"\\a^I", u8"Î"},{"\\a^O", u8"Ô"},{"\\a^U", u8"Û"},
  {"\\a\"E", u8"Ë"},{"\\a\"I", u8"Ï"},{"\\a'U", u8"Ú"},{"\\a`U", u8"Ù"},{"\\a,C", u8"Ç"},{"\\a\"u", u8"ü"},
  {"\\a'e", u8"é"},{"\\a^a", u8"â"},{"\\a\"a", u8"ä"},{"\\a`a", u8"à"},{"\\a*a", u8"å"},{"\\a,c", u8"ç"},
  {"\\a^e", u8"ê"},{"\\a'a", u8"á"},{"\\a\"e", u8"ë"},{"\\a`e", u8"è"},{"\\a\"i", u8"ï"},{"\\a^i", u8"î"},
  {"\\a`i", u8"ì"},{"\\a\"A", u8"Ä"},{"\\a*A", u8"Å"},{"\\a'E", u8"É"},{"\\a^o", u8"ô"},{"\\a\"o", u8"ö"},
  {"\\a/O", u8"Ø"},{"\\a/o", u8"ø"},{"\\a`o", u8"ò"},{"\\a^u", u8"û"},{"\\a`u", u8"ù"},{"\\a\"y", u8"ÿ"},
  {"\\a\"O", u8"Ö"},{"\\a\"U", u8"Ü"},{"\\a~n", u8"ñ"},{"\\a~N", u8"Ñ"},{"\\a'i", u8"í"},{"\\a'o", u8"ó"},
  {"\\a'u", u8"ú"},{"\\a/c", u8"¢"},{"\\a~A", u8"Ã"},{"\\a~a", u8"ã"},{"\\a~O", u8"Õ"},{"\\a~o", u8"õ"},
  {"\\a,A", u8"Ą"},{"\\a,a", u8"ą"},{"\\a'C", u8"Ć"},{"\\a'c", u8"ć"},{"\\a,E", u8"Ę"},{"\\a,e", u8"ę"},
  {"\\a/L", u8"Ł"},{"\\a/l", u8"ł"},{"\\a'N", u8"Ń"},{"\\a'n", u8"ń"},{"\\a'S", u8"Ś"},{"\\a's", u8"ś"},
  {"\\a'Z", u8"Ź"},{"\\a'z", u8"ź"},{"\\a.Z", u8"Ż"},{"\\a.z", u8"ż"}
};


static const std::unordered_map<std::string, const char *>   map_utf7_h {
  {"\\H01", u8""},{"\\H02", u8""},
  {"\\H03", u8"φ"},{"\\H04", u8"°"},{"\\Ha5", u8"£"},{"\\Ha6", u8"¥"},{"\\Ha7", u8""},{"\\Ha8", u8"ƒ"},
  {"\\Hb2", u8"°"},{"\\Hb4", u8"´"},{"\\Hb5", u8"½"},{"\\Hb6", u8"¼"},{"\\Hb7", u8"×"},{"\\Hb8", u8"÷"},
  {"\\Hb9", u8""},{"\\Hba", u8""},{"\\Hbb", u8"«"},{"\\Hbc", u8"»"},{"\\Hbd", u8""},{"\\Hbe", u8""},
  {"\\Hbf", u8"¯"},{"\\Hc0", u8""},{"\\Hc1", u8""},{"\\Hc2", u8""},{"\\Hc3", u8""},{"\\Hc4", u8""},
  {"\\Hc5", u8""},{"\\Hc6", u8""},{"\\Hc7", u8""},{"\\Hc8", u8""},{"\\Hc9", u8""},{"\\Hca", u8""},
  {"\\Hcb", u8""},{"\\Hcc", u8""},{"\\Hcd", u8"·"},{"\\Hce", u8"®"},{"\\Hcf", u8"©"},{"\\Hd0", u8"™"},
  {"\\Hd1", u8"†"},{"\\Hd2", u8"§"},{"\\Hf5", u8""},{"\\Hf6", u8""},{"\\Hf7", u8""},{"\\Hf8", u8""},
  {"\\Hf9", u8""},{"\\Hfa", u8""},{"\\Hfb", u8""},{"\\Hfc", u8""},{"\\Hfd", u8""},{"\\Hfe", u8""},
  {"\\Hff", u8""},{"\\H7f", u8""},{"\\H1a", u8"…"},{"\\H1b", u8"€"},{"\\H9d", u8"ª"},{"\\H9e", u8"º"},
  {"\\Hb1", u8"¨"},{"\\Ha9", u8"¡"},{"\\H1c", u8""}
};

static const std::unordered_map<std::string, const char *>  map_utf7_l	 {
  {"\\L-PI-", u8"¶"},{"\\LGAMM", u8"Γ"},{"\\LDELT", u8"Δ"},
  {"\\LTHET", u8"Θ"},{"\\LLAMB", u8"Λ"},{"\\LKSI-", u8"Ξ"},{"\\LC-PI", u8"Π"},{"\\LSIGM", u8"Σ"},{"\\LUPSI", u8"Υ"},
  {"\\LPHI-", u8"Φ"},{"\\LPSI-", u8"Ψ"},{"\\LOMEG", u8"Ω"},{"\\Lalph", u8"α"},{"\\Lbeta", u8"β"},{"\\Lgamm", u8"γ"},
  {"\\Ldelt", u8"δ"},{"\\Lepsi", u8"ε"},{"\\Ldzet", u8"ζ"},{"\\Leta-", u8"η"},{"\\Lthet", u8"θ"},{"\\Liota", u8"ι"},
  {"\\Lkapp", u8"κ"},{"\\Llamb", u8"λ"},{"\\L-MU-", u8"Μ"},{"\\Lnu--", u8"ν"},{"\\Lksi-", u8"ξ"},{"\\Lpi--", u8"π"},
  {"\\Lrho-", u8"ρ"},{"\\Lsigm", u8"σ"},{"\\Ltau-", u8"τ"},{"\\Lupsi", u8"υ"},{"\\Lphi-", u8"φ"},{"\\Lpsi-", u8"ψ"},
  {"\\Lomeg", u8"ω"},{"\\LINVLINV\?", u8"¿"},{"\\LBETA", u8"ß"},{"\\Lkhi-", u8"χ"},{"\\LsigE", u8"ς"},{"\\La--e", u8"æ"},
  {"\\LA--E", u8"Æ"},{"\\LALPH", u8"Α"},{"\\LC-BE", u8"Β"},{"\\LEPSI", u8"Ε"},{"\\LETA-", u8"Η"},{"\\LIOTA", u8"Ι"},
  {"\\LKAPP", u8"Κ"},{"\\LC-MU", u8"Μ"},{"\\LNU--", u8"Ν"},{"\\LOMIC", u8"Ο"},{"\\LRHO-", u8"Ρ"},{"\\LTAU-", u8"Τ"},
  {"\\LKHI-", u8"Χ"},{"\\LDZET", u8"Ζ"},{"\\Lomic", u8"ο"},{"\\Lepsp", u8"έ"},{"\\Lalpp", u8"ά"},{"\\Liotp", u8"ί"},
  {"\\Lomip", u8"ό"},{"\\Lupsp", u8"ύ"},{"\\Lomep", u8"ώ"},{"\\LOMIP", u8"Ό"},{"\\LALPP", u8"Ά"},{"\\LEPSP", u8"Έ"}, {"\\Lmu--", u8"μ"}};





void fix_format(std::string& file, size_t &output_index)
{
  static const std::string& search_return_fld   {"\nFLD="} ;
  static const std::string& replace_escape_fld  {"\033FLD="} ;
  static const std::string& search_nocst_equal  {"NO_CST="};
  static const std::string& search_bogus_ref    {"\n\nREF="};
  static const std::string& replace_fixed_ref   {"\nREF="};
  static const std::string& replace_nocst_equal {"NOCST=-"};
  static const std::string& search_empty_nocst  {"NO_CST"};
  static const std::string& replace_empty_nocst {"NOCST=-undefined"};

  size_t pos =output_index ;
  while ((pos = file.find(search_nocst_equal, pos)) != std::string::npos) {
    file.replace(pos, search_nocst_equal.length(), replace_nocst_equal);
    pos += replace_nocst_equal.length();
  }
  pos = output_index;
  while ((pos = file.find(search_empty_nocst, pos)) != std::string::npos) {
    file.replace(pos, search_empty_nocst.length(), replace_empty_nocst);
    pos += replace_empty_nocst.length();
  }
  pos = output_index;
  while ((pos = file.find(search_bogus_ref, pos)) != std::string::npos) {
    file.replace(pos, search_bogus_ref.length(), replace_fixed_ref);
    pos += replace_fixed_ref.length();
  }
  pos =output_index;
  while ((pos = file.find(search_return_fld, pos)) != std::string::npos) {
    file.replace(pos, search_return_fld.length(), replace_escape_fld);
    pos += replace_escape_fld.length();
  }
}

void
canonize_file_format( std::string& input,std::string& output, bool application)
{
  const size_t input_lenght   {input.size()};
  size_t previous_input_index {0};
  size_t input_index          {0};
  bool in_comment             {false};
  size_t output_index         {0};

  input += "/\023"; // add XOFF as a sentinel.

  do {
    input_index = input.find( '/', input_index);

    if (! in_comment && (previous_input_index < input_index)) {
      output_index = output.size();
      output.append(input.substr(previous_input_index, input_index - previous_input_index));
      fix_format(output, output_index );
      previous_input_index = input_index + 1;
    }
    if (in_comment && (input.at(input_index - 1) == '*')) {
      in_comment = false;
      previous_input_index = ++input_index;
    } else if (input.at(input_index + 1) == '/') {
      input_index = input.find( '\n', previous_input_index);
      previous_input_index = input_index;
    } else if (input.at(input_index + 1) == '*') {
      in_comment = true;input_index+=2;previous_input_index= input_index;
    }
    else if (input.at(input_index + 1) == '/') {
      input_index = input.find( '\n', previous_input_index);
      previous_input_index = input_index;
    } else if (input.at(input_index + 1) == '\023') {
      if (! application && output.find_first_of("FLD=") != std::string::npos)
        output.erase(0, output.find_first_of("FLD="));
      return;
    } else
      input_index++;
  } while (input_index < input_lenght);


}
struct pattern_utf7
{
  size_t pattern_index  {0};
  size_t pattern_lenght {4};
  const std::unordered_map<std::string, const char *>  *map_pattern {nullptr};
  std::string ref {};
  inline bool is_a_match(const std::string &input,size_t input_index)
  {
    if (input.at(input_index ) == '\\') {
      if (input.at(input_index +1 ) == 'x') {
	map_pattern = &map_utf7_x;
	ref = input.substr(input_index,  4);
	return true;
      } else if (input.at(input_index +1) == 'a') {
	map_pattern = &map_utf7_a;
	ref = input.substr(input_index, 4);
	return true;
      }  else if (input.at(input_index +1) == 'H') {
	map_pattern = &map_utf7_h;
	ref = input.substr(input_index,  4);
	return true;
      } else if (input.at(input_index +1) == 'L') {
	map_pattern = &map_utf7_l;
	ref = input.substr(input_index, 6);
	return true;
      }
    }
    return false;
  }
  inline bool replace(std::string &input,size_t input_index )
  {
    //auto it {map_pattern->find(ref)};
    const std::unordered_map<std::string, const char *>::const_iterator got = map_pattern->find(ref);
    if ( got != end(*map_pattern) )
      input.replace (input_index , ref.size(), got->second);
    return false;
  }
};


void
from_alecode2utf8(std::string &input)
{
  pattern_utf7 pattern;
  size_t input_index {0};
  size_t q;

  for(input_index = input.find( '\\', input_index), q=0; input_index!=input.npos; input_index  = q)
    {
      if (((input_index +3) < input.size()) && (pattern.is_a_match(input, input_index)))
	           pattern.replace(input, input_index);
      q=input.find('\\', input_index+1);
    }
}


size_t
string_glyph_length(const std::string &input)
{
  // Count all first-bytes (the ones that don't match 10xxxxxx).'
  size_t count_head_bytes {0};
  for (const auto byte: input)
    count_head_bytes += ( byte & 0xc0) != 0x80;
  return  count_head_bytes;
}

size_t string_byte_size(const std::string &input)
{
  size_t count_head_bytes {string_glyph_length(input)};
  size_t reserve_byte_size {0};
  // 3BA_29000_0680_PEZZA OXE Strings Management - Developer Guide page 6.
  switch(count_head_bytes) {
  case 1 :   reserve_byte_size = 3; break;
  case 2 :   reserve_byte_size = 6; break;
  case 3 :   reserve_byte_size = 9; break;
  case 4 :   reserve_byte_size = 12; break;
  case 5 :   reserve_byte_size = 14; break;
  default :  reserve_byte_size = count_head_bytes*2 + 3; break;
  }
  return reserve_byte_size;
}









std::vector<std::string>
split_docfile(const std::string& input) {
  static const  std::regex& re {"(\033)"} ;

  const std::sregex_token_iterator
    first{input.begin(), input.end(), re, -1},
    last;
    return {first, last};
}

std::vector<std::string>
split_appfile(const std::string& input) {
  static const  std::regex& re {"(\n)"};

  std::sregex_token_iterator
    first{input.begin(), input.end(), re, -1},
    last;
    return {first, last};
}









#if 0

inline std::vector<std::string>
split_tslfile(const std::string& input) {
  static const std::regex& re{"(\033)"};

  std::sregex_token_iterator
    first{input.begin(), input.end(), re, -1},
    last;
    return {first, last};
}
#endif




/*
char sep = ' ';
std::string s="1 This is an example";

for(size_t p=0, q=0; p!=s.npos; p=q)
  std::cout << s.substr(p+(p!=0), (q=s.find(sep, p+1))-p-(p!=0)) << std::endl;
*/
#if MERY
void fix_nocst(std::string& file)
{
   static const  std::string& search_return_fld  {"\nFLD="} ;

   static const  std::string& replace_escape_fld  {"\033FLD="} ;
   static  const std::string& search_nocst_equal  {"NO_CST="};
   static  const std::string& search_bogus_ref    {"\n\nREF="};
   static  const std::string& replace_fixed_ref   {"\nREF="};
   static  const std::string& replace_nocst_equal {"NOCST=-"};
   static  const std::string& search_empty_nocst  {"NO_CST"};
   static  const std::string& replace_empty_nocst {"NOCST=-undefined"};
   //static  const std::regex&  regex_comment_cee   {R"(/\*([^*]|[\r\n]|(\*+([^*\/]|[\r\n])))*\*/)"};
   static  const std::regex&  regex_comment_ceepp {R"(//[^\n]*)" };
   static  const std::string& replace_empty_string {""};

  size_t pos = 0;
  while ((pos = file.find(search_nocst_equal, pos)) != std::string::npos) {
    file.replace(pos, search_nocst_equal.length(), replace_nocst_equal);
    pos += replace_nocst_equal.length();
  }

  pos = 0;
  while ((pos = file.find(search_empty_nocst, pos)) != std::string::npos) {
    file.replace(pos, search_empty_nocst.length(), replace_empty_nocst);
    pos += replace_empty_nocst.length();
  }

  // file = regex_replace(file, regex_comment_cee, replace_empty_string); bug
   file = regex_replace(file, regex_comment_ceepp, replace_empty_string);

    pos = 0;
  while ((pos = file.find(search_bogus_ref, pos)) != std::string::npos) {
    file.replace(pos, search_bogus_ref.length(), replace_fixed_ref);
    pos += replace_fixed_ref.length();
  }
     pos = 0;
  while ((pos = file.find(search_return_fld, pos)) != std::string::npos) {
    file.replace(pos, search_return_fld.length(), replace_escape_fld);
    pos += replace_escape_fld.length();
  }

}

#endif
