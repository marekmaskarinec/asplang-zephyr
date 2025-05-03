//
// Asp compiler main.
//

#include "lexer.h"
#include "compiler.h"
#include "executable.hpp"
#include "symbol.hpp"
#include "asp.h"
#include "search-path.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <string>
#include <cstring>
#include <memory>
#include <cstdlib>
#include <cerrno>

#if !defined ASP_COMPILER_VERSION_MAJOR || \
    !defined ASP_COMPILER_VERSION_MINOR || \
    !defined ASP_COMPILER_VERSION_PATCH || \
    !defined ASP_COMPILER_VERSION_TWEAK
#error ASP_COMPILER_VERSION_* macros undefined
#endif
#ifndef COMMAND_OPTION_PREFIXES
#error COMMAND_OPTION_PREFIXES macro undefined
#endif
#ifndef FILE_NAME_SEPARATORS
#error FILE_NAME_SEPARATORS macro undefined
#endif

static const double DefaultCodeSizeWarningRatio = 0.8;

// Lemon parser.
extern "C" {
void *ParseAlloc(void *(*malloc)(size_t), Compiler *);
void Parse(void *parser, int yymajor, Token *yyminor);
void ParseFree(void *parser, void (*free)(void *));
void ParseTrace(FILE *, const char *prefix);
}

using namespace std;

static void Usage()
{
    cerr
        << "Usage:      aspc [OPTION]... ["
        << COMMAND_OPTION_PREFIXES[0] << COMMAND_OPTION_PREFIXES[0]
        << "] [SPEC] SCRIPT\n"
        << " or         aspc [OPTION]... ["
        << COMMAND_OPTION_PREFIXES[0] << COMMAND_OPTION_PREFIXES[0]
        << "] SCRIPT [SPEC]\n"
        << "\n"
        << "Compile the Asp script source file SCRIPT (*.asp)."
        << " The application specification\n"
        << "file (*.aspec) may be given as SPEC."
        << " If omitted, an app.aspec file in the same\n"
        << "directory as the source file is used if present."
        << " Otherwise, the value of the\n"
        << "ASP_SPEC_FILE environment variable is used if defined."
        << " If the application\n"
        << "specification file is not specified in any of these ways,"
        << " the command ends\n"
        << "in error.\n"
        << "\n"
        << "Use " << COMMAND_OPTION_PREFIXES[0] << COMMAND_OPTION_PREFIXES[0]
        << " before the first argument if it starts with an option prefix.\n"
        << "\n"
        << "Options";
    if (strlen(COMMAND_OPTION_PREFIXES) > 1)
    {
        cerr << " (may be prefixed by";
        for (unsigned i = 1; i < strlen(COMMAND_OPTION_PREFIXES); i++)
        {
            if (i != 1)
            {
                if (i == strlen(COMMAND_OPTION_PREFIXES) - 1)
                    cerr << " or";
                else
                    cerr << ',';
            }
            cerr << ' ' << COMMAND_OPTION_PREFIXES[i];
        }
        cerr << " instead of " << COMMAND_OPTION_PREFIXES[0] << ')';
    }
    cerr
        << ":\n"
        << COMMAND_OPTION_PREFIXES[0]
        << "c SIZE     Maximum code size in bytes. An error is raised if the"
        << " executable's\n"
        << "            code size exceeds the given value. The default (and"
        << " maximum) is\n"
        << "            " << Executable::MaxCodeSize << " (";
    {
        auto oldFlags = cerr.flags();
        cerr << showbase << hex << Executable::MaxCodeSize;
        cerr.flags(oldFlags);
    }
    cerr
        << "). This value is also used in conjunction with\n"
        << "            the code size warning level ("
        << COMMAND_OPTION_PREFIXES[0] << "w option).\n"
        << COMMAND_OPTION_PREFIXES[0]
        << "h          Print usage information and exit.\n"
        << COMMAND_OPTION_PREFIXES[0]
        << "o FILE     Write outputs to FILE.* instead of basing file names"
        << " on the SCRIPT\n"
        << "            file name. If FILE ends with .aspe, its base name is"
        << " used instead.\n"
        << "            If FILE ends with " << FILE_NAME_SEPARATORS[0]
        << ", SCRIPT.* will be written into the directory\n"
        << "            given by FILE. In this case, the directory must"
        << " already exist.\n"
        << COMMAND_OPTION_PREFIXES[0]
        << "q          Quiet. Don't output usual compiler information.\n"
        << COMMAND_OPTION_PREFIXES[0]
        << "v          Print version information and exit.\n"
        << COMMAND_OPTION_PREFIXES[0]
        << "w PERCENT  Code size warning level, as a percentage of the"
        << " maximum (i.e., the\n"
        << "            value given by the " << COMMAND_OPTION_PREFIXES[0]
        << "c option or its default). A warning will be\n"
        << "            issued if the code size exceeds the given amount. The"
        << " default level\n"
        << "            is " << (DefaultCodeSizeWarningRatio * 100) << "%.\n";
}

static int main1(int argc, char **argv);
int main(int argc, char **argv)
{
    try
    {
        return main1(argc, argv);
    }
    catch (const string &e)
    {
        cerr << "Error: " << e << endl;
    }
    catch (...)
    {
        cerr << "Unknown error" << endl;
    }
    return EXIT_FAILURE;
}
static int main1(int argc, char **argv)
{
    // Process command line options.
    bool quiet = false, reportVersion = false;
    string outputBaseName;
    uint32_t maxCodeSize = Executable::MaxCodeSize;
    double codeSizeWarningRatio = DefaultCodeSizeWarningRatio;
    for (; argc >= 2; argc--, argv++)
    {
        string arg1 = argv[1];
        string prefix = arg1.substr(0, 1);
        auto prefixIndex =
            string(COMMAND_OPTION_PREFIXES).find_first_of(prefix);
        if (prefixIndex == string::npos)
            break;
        string option = arg1.substr(1);
        if (option == string(1, COMMAND_OPTION_PREFIXES[prefixIndex]))
        {
            argc--; argv++;
            break;
        }

        if (option == "?" || option == "h")
        {
            Usage();
            return 0;
        }
        else if (option == "c")
        {
            if (argc <= 2)
            {
                Usage();
                return 1;
            }

            string value = (++argv)[1];
            argc--;
            char *p;
            long size = strtol(value.c_str(), &p, 0);
            if (*p != 0 || size <= 0 || size > Executable::MaxCodeSize)
            {
                auto oldFlags = cerr.flags();
                cerr
                    << "Invalid max code size: " << value
                    << " (must be a positive integer up to "
                    << Executable::MaxCodeSize
                    << ", i.e., " << showbase << hex << Executable::MaxCodeSize
                    << ')' << endl;
                cerr.flags(oldFlags);
                return 1;
            }
            maxCodeSize = static_cast<uint32_t>(size);
        }
        else if (option == "o")
        {
            if (argc <= 2)
            {
                Usage();
                return 1;
            }

            outputBaseName = (++argv)[1];
            argc--;
        }
        else if (option == "q" || option == "s")
            quiet = true;
        else if (option == "v")
            reportVersion = true;
        else if (option == "w")
        {
            if (argc <= 2)
            {
                Usage();
                return 1;
            }

            string value = (++argv)[1];
            argc--;
            char *p;
            double percentage = strtod(value.c_str(), &p);
            if (*p != 0 || percentage < 0 || percentage > 100)
            {
                cerr
                    << "Invalid code size warning level: " << value
                    << " (must be a number between 0 and 100)" << endl;
                return 1;
            }
            codeSizeWarningRatio = percentage * 0.01;
        }
        else
        {
            cerr << "Invalid option: " << arg1 << endl;
            return 1;
        }
    }

    // Report version information and exit if requested.
    if (reportVersion)
    {
        cout
            << "Asp compiler version "
            << ASP_COMPILER_VERSION_MAJOR << '.'
            << ASP_COMPILER_VERSION_MINOR << '.'
            << ASP_COMPILER_VERSION_PATCH << '.'
            << ASP_COMPILER_VERSION_TWEAK
            << endl;
        return 0;
    }

    // Obtain input file name(s).
    if (argc < 2 || argc > 3)
    {
        Usage();
        return 1;
    }
    string specFileName, mainModuleFileName;
    size_t mainModuleSuffixPos;
    struct InputSpec
    {
        string suffix;
        string *fileName;
        size_t *suffixPos;
    };
    struct InputSpec inputs[] =
    {
        {".aspec", &specFileName, nullptr},
        {".asp", &mainModuleFileName, &mainModuleSuffixPos},
    };
    for (int argi = 1; argi < argc; argi++)
    {
        string fileName(argv[argi]);
        bool accepted = false;
        for (auto &&input: inputs)
        {
            if (fileName.size() <= input.suffix.size())
                continue;
            auto suffixPos = fileName.size() - input.suffix.size();
            if (fileName.substr(suffixPos) == input.suffix)
            {
                if (!input.fileName->empty())
                {
                    cerr
                        << "Error: Multiple files of the same type specified"
                        << endl;
                    Usage();
                    return 1;
                }

                accepted = true;
                *input.fileName = fileName;
                if (input.suffixPos != nullptr)
                    *input.suffixPos = suffixPos;
            }
        }
        if (!accepted)
        {
            cerr << "Error: Unrecognized input file type" << endl;
            Usage();
            return 1;
        }
    }

    // If the source file is not defined at this point, issue an error and
    // exit.
    if (mainModuleFileName.empty())
    {
        cerr << "Error: Source file not specified" << endl;
        Usage();
        return 1;
    }

    // Split the main module file name into its constituent parts.
    auto mainModuleDirectorySeparatorPos = mainModuleFileName.find_last_of
        (FILE_NAME_SEPARATORS);
    size_t baseNamePos = mainModuleDirectorySeparatorPos == string::npos ?
        0 : mainModuleDirectorySeparatorPos + 1;
    auto mainModuleDirectoryName = mainModuleFileName.substr(0, baseNamePos);
    auto mainModuleBaseFileName = mainModuleFileName.substr(baseNamePos);

    // If the application specification is not specified, check for the
    // existence of a fixed-named file in the same directory as the source
    // file.
    if (specFileName.empty())
    {
        static const string localSpecBaseFileName = "app.aspec";
        string localSpecFileName = mainModuleDirectoryName;
        if (!localSpecFileName.empty() &&
            strchr(FILE_NAME_SEPARATORS, localSpecFileName.back()) == nullptr)
            localSpecFileName += FILE_NAME_SEPARATORS[0];
        localSpecFileName += localSpecBaseFileName;
        ifstream localSpecStream(localSpecFileName, ios::binary);
        if (localSpecStream)
            specFileName = localSpecFileName;
    }

    // If the application specification is still not defined, resort to the
    // environment variable.
    if (specFileName.empty())
    {
        const char *envSpecFileNameString = getenv("ASP_SPEC_FILE");
        if (envSpecFileNameString != nullptr)
            specFileName = envSpecFileNameString;
    }

    // If the application specification is not defined at this point, issue an
    // error and exit.
    if (specFileName.empty())
    {
        cerr
            << "Error: Application specification not specified"
            << " and no default found" << endl;
        return 2;
    }

    // Open the application specification file.
    ifstream specStream(specFileName, ios::binary);
    if (!specStream)
    {
        cerr
            << "Error opening " << specFileName
            << ": " << strerror(errno) << endl;
        return 2;
    }
    if (!quiet)
        cout << "Using " << specFileName << endl;

    // Determine the base name of all output files.
    static string executableSuffix = ".aspe";
    if (outputBaseName.size() > executableSuffix.size() &&
        outputBaseName.substr(outputBaseName.size() - executableSuffix.size())
        == executableSuffix)
        outputBaseName.erase(outputBaseName.size() - executableSuffix.size());
    string baseName =
        outputBaseName.empty() ?
            mainModuleBaseFileName.substr
                (0, mainModuleSuffixPos - baseNamePos) :
        strchr(FILE_NAME_SEPARATORS, outputBaseName.back()) == nullptr ?
            outputBaseName :
            outputBaseName +
            mainModuleBaseFileName.substr
                (0, mainModuleSuffixPos - baseNamePos);

    // Open output executable.
    string executableFileName = baseName + executableSuffix;
    ofstream executableStream(executableFileName, ios::binary);
    if (!executableStream)
    {
        cerr
            << "Error creating " << executableFileName
            << ": " << strerror(errno) << endl;
        return 2;
    }

    // Open output listing.
    static string listingSuffix = ".lst";
    string listingFileName = baseName + listingSuffix;
    ofstream listingStream(listingFileName);
    if (!listingStream)
    {
        cerr
            << "Error creating " << listingFileName
            << ": " << strerror(errno) << endl;
        return 2;
    }

    // Open output source info file.
    static string sourceInfoSuffix = ".aspd";
    string sourceInfoFileName = baseName + sourceInfoSuffix;
    ofstream sourceInfoStream(sourceInfoFileName, ios::binary);
    if (!sourceInfoStream)
    {
        cerr
            << "Error creating " << sourceInfoFileName
            << ": " << strerror(errno) << endl;
        return 2;
    }

    // Predefine symbols for main module and application functions.
    SymbolTable symbolTable;
    Executable executable(symbolTable);
    Compiler compiler(cerr, symbolTable, executable);
    compiler.LoadApplicationSpec(specStream);
    compiler.AddModuleFileName(mainModuleBaseFileName);

    // Prepare to search for imported module files.
    vector<string> searchPath;
    const char *includePathString = getenv("ASP_INCLUDE");
    if (includePathString != nullptr)
    {
        auto includePath = SearchPath(includePathString);
        searchPath.insert
            (searchPath.end(), includePath.begin(), includePath.end());
    }
    if (searchPath.empty())
        searchPath.emplace_back();

    // Compile main module and any other modules that are imported.
    bool errorDetected = compiler.ErrorCount() > 0;
    while (!errorDetected)
    {
        string moduleName = compiler.NextModule();
        if (moduleName.empty())
            break;
        static string sourceSuffix = ".asp";
        string moduleFileName = moduleName + sourceSuffix;

        // Open module file.
        auto moduleStream = unique_ptr<istream>();
        if (moduleFileName == mainModuleBaseFileName)
        {
            // Open specified main module file.
            auto stream = unique_ptr<istream>
                (new ifstream(mainModuleFileName));
            if (*stream)
                moduleStream = move(stream);
        }
        else
        {
            // Search for the module file using the search path.
            for (auto &&directory: searchPath)
            {
                // For an empty entry, use the main module's directory (which,
                // it may be noted, may also be empty).
                if (directory.empty())
                    directory = mainModuleDirectoryName;

                // Construct a path name for the module file.
                if (!directory.empty() &&
                    strchr(FILE_NAME_SEPARATORS, directory.back()) == nullptr)
                    directory += FILE_NAME_SEPARATORS[0];
                auto modulePathName = directory + moduleFileName;

                // Attempt opening the module file.
                auto stream = unique_ptr<istream>
                    (new ifstream(modulePathName));
                if (*stream)
                {
                    // Issue a warning if the module's name matches an
                    // application module.
                    if (compiler.IsAppModule(moduleName))
                    {
                        cerr
                            << "WARNING: Importing application module "
                            << moduleName << " instead of "
                            << modulePathName << " found on path" << endl;
                        stream.reset();
                        continue;
                    }

                    moduleStream = move(stream);
                    break;
                }
            }
        }
        if (moduleStream == nullptr)
        {
            // Ignore failure to find an application module in the path.
            if (compiler.IsAppModule(moduleName))
                continue;

            cerr
                << "Error opening " << moduleFileName
                << ": " << strerror(errno) << endl;
            errorDetected = true;
            break;
        }
        Lexer lexer(*moduleStream, moduleFileName);

        #ifdef ASP_COMPILER_DEBUG
        cout << "Parsing module " << moduleFileName << "..." << endl;
        ParseTrace(stdout, "Trace: ");
        #endif

        void *parser = ParseAlloc(malloc, &compiler);

        Token *token;
        do
        {
            token = lexer.Next();
            string error;
            switch (token->type)
            {
                default:
                    break;
                case -1:
                    error = "Bad token encountered";
                    break;
                case TOKEN_UNEXPECTED_INDENT:
                    error = "Unexpected indentation";
                    break;
                case TOKEN_MISSING_INDENT:
                    error = "Missing indentation";
                    break;
                case TOKEN_MISMATCHED_UNINDENT:
                    error = "Mismatched indentation";
                    break;
                case TOKEN_INCONSISTENT_WS:
                    error = "Inconsistent whitespace in indentation";
                    break;
            }
            if (!error.empty())
            {
                cerr
                    << token->sourceLocation.fileName << ':'
                    << token->sourceLocation.line << ':'
                    << token->sourceLocation.column
                    << ": " << error;
                if (!token->s.empty())
                    cerr << ": '" << token->s << '\'';
                if (!token->error.empty())
                    cerr << ": " << token->error;
                cerr << endl;

                delete token;
                errorDetected = true;
                break;
            }

            Parse(parser, token->type, token);
            if (compiler.ErrorCount() > 0)
                errorDetected = true;

        } while (!errorDetected && token->type != 0);

        ParseFree(parser, free);
    }

    compiler.Finalize();
    auto finalCodeSize = executable.FinalCodeSize();
    if (errorDetected || finalCodeSize > maxCodeSize)
    {
        if (finalCodeSize > maxCodeSize)
        {
            cerr
                << "ERROR: Code size exceeds the maximum: "
                << finalCodeSize << " vs. max " << maxCodeSize << " bytes"
                << endl;
        }

        cerr << "Ended in ERROR" << endl;

        // Remove output files.
        executableStream.close();
        remove(executableFileName.c_str());
        listingStream.close();
        remove(listingFileName.c_str());
        sourceInfoStream.close();
        remove(sourceInfoFileName.c_str());
        return 4;
    }

    // Write the code.
    executable.Write(executableStream);
    auto executableByteCount = executableStream.tellp();
    executableStream.close();
    if (!executableStream)
    {
        cerr
            << "Error writing " << executableFileName
            << ": " << strerror(errno) << endl;
        remove(executableFileName.c_str());
    }

    // Write the listing.
    executable.WriteListing(listingStream);
    listingStream.close();
    if (!listingStream)
    {
        cerr
            << "Error writing " << listingFileName
            << ": " << strerror(errno) << endl;
    }

    // Write the source info file.
    executable.WriteSourceInfo(sourceInfoStream);
    sourceInfoStream.close();
    if (!sourceInfoStream)
    {
        cerr
            << "Error writing " << sourceInfoFileName
            << ": " << strerror(errno) << endl;
    }

    // Indicate any error writing the code.
    if (!executableStream)
        return 5;
    if (!listingStream || !sourceInfoStream)
        return 6;

    // Report statistics.
    if (executableStream && !quiet)
    {
        cout
            << executableFileName << ": "
            << executableByteCount << " bytes" << endl;

        // Warn for executables nearing maximum size.
        double codeSizeRatio = (double)finalCodeSize / maxCodeSize;
        if (codeSizeRatio >= codeSizeWarningRatio)
        {
            auto oldFlags = cout.flags();
            auto oldPrecision = cout.precision();
            cout
                << "WARNING: Code size is at "
                << fixed << setprecision(1) << (codeSizeRatio * 100)
                << "% of maximum: "
                << (maxCodeSize - executable.FinalCodeSize())
                << " bytes remaining" << endl;
            cout.flags(oldFlags);
            cout.precision(oldPrecision);
        }
    }

    return 0;
}
