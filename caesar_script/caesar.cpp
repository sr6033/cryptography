///
/// @file
/// @author Shubham Rath
/// @copyright MIT/Expat License.
/// @brief Caesar Cipher implementation.
///
/// This program demonstrates proficient usage of C++ features to implement the Caesar cipher.
///
/// http://en.wikipedia.org/wiki/Caesar_cipher
///

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>

///
/// @brief Encrypts letters in the range [`src_begin`, `src_end`) using `shift` and writes to `dest_begin`.
/// @param src_begin    source range beginning
/// @param src_end      source range end
/// @param dest_begin   destination range beginning
/// @param shift        desired shift of the alphabet
/// @note Internally, the shifting value will become `shift` modulo alphabet-length.
/// @return `OutputIterator` to the element past the last element written.
///

template <typename InputIterator, typename OutputIterator>
OutputIterator caesar_cipher(InputIterator src_begin, InputIterator src_end, OutputIterator dest_begin, int shift)
{
    const std::string ab("abcdefghijklmnopqrstuvwxyz"); // AlphaBet
    std::string rot_ab(ab); // ROTated AlphaBet

    shift %= static_cast<int> (ab.length()); // bring the shift within the length of the alphabet

    if (shift < 0)
        std::rotate(rot_ab.rbegin(), rot_ab.rbegin() - shift, rot_ab.rend());
    else
        std::rotate(rot_ab.begin(), rot_ab.begin() + shift, rot_ab.end());

    return std::transform(src_begin, src_end, dest_begin, [ab, rot_ab](unsigned char c) -> char {
        if (std::isalpha(c))
        {
            if (std::isupper(c))
                return std::toupper(rot_ab.at(ab.find(std::tolower(c))));

            return rot_ab.at(ab.find(c));
        }

        return c;
    });
}

///
/// @brief Prints usage information and a custom message.
/// @param s    custom message to be printed
///

void print_usage(const std::string &s = "")
{
    std::cerr << "Usage example:\n";
    std::cerr << "\tprogram.exe input_file output_file shift\n\n";
    std::cerr << "Where `input_file' and `output_file' are distinct files, and\n";
    std::cerr << "`shift' is an integer representing the desired alphabet shift.\n";

    if (!s.empty())
        std::cerr << "\nERROR: " << s << '\n';

    std::cerr << std::endl;
}

#ifdef DEBUGGING

#include <cassert>
#include <cstring>
#include <list>
#include <memory>
#include <vector>

///
/// @brief Debugging program entry point.
///

int main()
{
    std::string s("Hello, World!");

    caesar_cipher(s.begin(), s.end(), s.begin(), 4);
    std::cout << s << '\n';
    assert(s == "Lipps, Asvph!");

    caesar_cipher(s.begin(), s.end(), s.begin(), -4);
    std::cout << s << '\n';
    assert(s == "Hello, World!");

    const std::vector<char> vc{'D', 'b', 'f', 't', 'b', 's'};
    std::list<char>         lc(vc.size());

    caesar_cipher(vc.begin(), vc.end(), lc.begin(), -1);
    for (char c: lc) std::cout << c;
    std::cout << '\n';
    assert(lc == std::list<char>({'C', 'a', 'e', 's', 'a', 'r'}));

    const char              ca[]{"Sqjzanxwn!"};
    std::unique_ptr<char[]> upca(new char[sizeof ca]);

    caesar_cipher(std::begin(ca), std::end(ca), upca.get(), 4);
    std::cout << upca.get() << '\n';
    assert(std::strcmp(upca.get(), "Wunderbar!") == 0);

    std::string s2("Wkh Txlfn Eurzq Ira Mxpsv Ryhu Wkh Odcb Grj");

    caesar_cipher(s2.begin(), s2.end(), s2.begin(), -55); // note: -55 modulo 26 == -3
    std::cout << s2 << '\n';
    assert(s2 == "The Quick Brown Fox Jumps Over The Lazy Dog");
    std::cout << std::endl;
}

#else

///
/// @brief Actual program entry point.
/// @param argc             number of command line arguments
/// @param [in] argv        array of command line arguments
/// @retval EXIT_FAILURE    for failed operation
/// @retval EXIT_SUCCESS    for successful operation
///

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        print_usage("Wrong number of arguments.");
        return EXIT_FAILURE;
    }

    std::ifstream input_file(argv[1], std::ios_base::binary);

    if (!input_file.is_open())
    {
        print_usage(std::string("input_file `") + argv[1] + "' could not be opened.");
        return EXIT_FAILURE;
    }

    std::ofstream output_file(argv[2], std::ios_base::binary);

    if (!output_file.is_open())
    {
        print_usage(std::string("output_file `") + argv[2] + "' could not be opened.");
        return EXIT_FAILURE;
    }

    int shift;

    if (!(std::istringstream(argv[3]) >> shift))
    {
        print_usage("shift conversion error.");
        return EXIT_FAILURE;
    }

    try
    {
        input_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);
        output_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);

        std::istreambuf_iterator<char> src_begin(input_file);
        std::istreambuf_iterator<char> src_end;
        std::ostreambuf_iterator<char> dest_begin(output_file);

        caesar_cipher(src_begin, src_end, dest_begin, shift);
    }
    catch (const std::ios_base::failure &f)
    {
        print_usage(std::string("File input/output failure: ") + f.what() + '.');
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif
