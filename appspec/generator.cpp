//
// Asp application specification generator implementation.
//

#include "generator.h"
#include "app.h"
#include "function.hpp"
#include "symbols.h"
#include "grammar.hpp"
#include <iostream>
#include <sstream>
#include <cctype>

using namespace std;

Generator::Generator
    (ostream &errorStream,
     const string &baseFileName) :
    errorStream(errorStream),
    symbolTable(true),
    moduleIdTable(false),
    baseFileName(baseFileName),
    baseName(baseFileName)
{
    // Reserve module ID zero for the system module.
    moduleIdTable.Symbol(AspSystemModuleName);

    // Deal with invalid variable name characters in the file name.
    for (string::iterator si = baseName.begin();
         si != baseName.end();
         si++)
    {
        auto c = *si;
        if (!isalnum(c) && c != '_')
            *si = '_';
    }

    // Create a space for the system module. Note that an empty string is used
    // as the key rather than the name 'sys' so that this will be the first
    // entry, and therefore be written first to the compiler spec.
    currentModuleDefinitions = &definitions.emplace
        ("", map<string, shared_ptr<NonTerminal> >()).first->second;
}

unsigned Generator::ErrorCount() const
{
    return errorCount;
}

void Generator::CurrentSource
    (const string &sourceFileName,
     const string &moduleName,
     bool newFile, bool isLibrary,
     const SourceLocation &sourceLocation)
{
    this->newFile = newFile;
    this->isLibrary = isLibrary;
    currentSourceFileName = sourceFileName;
    currentModuleName = moduleName;
    currentModuleDefinitions = &definitions.find(moduleName)->second;
    currentSourceLocation = sourceLocation;
}

bool Generator::IsLibrary() const
{
    return isLibrary;
}

const string &Generator::CurrentSourceFileName() const
{
    return currentSourceFileName;
}

const string &Generator::CurrentModuleName() const
{
    return currentModuleName;
}

SourceLocation Generator::CurrentSourceLocation() const
{
    return currentSourceLocation;
}

#define DEFINE_ACTION(...) DEFINE_ACTION_N(__VA_ARGS__, \
    DEFINE_ACTION_3, ~, \
    DEFINE_ACTION_2, ~, \
    DEFINE_ACTION_1, ~, ~, ~, ~)(__VA_ARGS__)
#define DEFINE_ACTION_N(_1, _2, _3, _4, _5, _6, _7, _8, arg, ...) arg
#define DEFINE_ACTION_1(action, ReturnType, type, param) \
    extern "C" ReturnType action(Generator *generator, type param) \
    {DO_ACTION(generator->action(param));} \
    ReturnType Generator::action(type param)
#define DEFINE_ACTION_2(action, ReturnType, t1, p1, t2, p2) \
    extern "C" ReturnType action(Generator *generator, \
        t1 p1, t2 p2) \
    {DO_ACTION(generator->action(p1, p2));} \
    ReturnType Generator::action \
        (t1 p1, t2 p2)
#define DEFINE_ACTION_3(action, ReturnType, t1, p1, t2, p2, t3, p3) \
    extern "C" ReturnType action(Generator *generator, \
        t1 p1, t2 p2, t3 p3) \
    {DO_ACTION(generator->action(p1, p2, p3));} \
    ReturnType Generator::action \
        (t1 p1, t2 p2, t3 p3)
#define DO_ACTION(action) \
    auto result = (action); \
    if (result && result->sourceLocation.line != 0) \
        generator->currentSourceLocation = result->sourceLocation; \
    return result;

#define DEFINE_UTIL(...) DEFINE_UTIL_N(__VA_ARGS__, \
    DEFINE_UTIL_3, ~, \
    DEFINE_UTIL_2, ~, \
    DEFINE_UTIL_1, ~, ~, ~, ~)(__VA_ARGS__)
#define DEFINE_UTIL_N(_1, _2, _3, _4, _5, _6, _7, _8, arg, ...) arg
#define DEFINE_UTIL_1(action, ReturnType, type, param) \
    extern "C" ReturnType action(Generator *generator, type param) \
    {generator->action(param);} \
    ReturnType Generator::action(type param)
#define DEFINE_UTIL_2(action, ReturnType, t1, p1, t2, p2) \
    extern "C" ReturnType action(Generator *generator, \
        t1 p1, t2 p2) \
    {generator->action(p1, p2);} \
    ReturnType Generator::action \
        (t1 p1, t2 p2)
#define DEFINE_UTIL_3(action, ReturnType, t1, p1, t2, p2, t3, p3) \
    extern "C" ReturnType action(Generator *generator, \
        t1 p1, t2 p2, t3 p3) \
    {generator->action(p1, p2, p3);} \
    ReturnType Generator::action \
        (t1 p1, t2 p2, t3 p3)

DEFINE_ACTION
    (DeclareAsLibrary, NonTerminal *, int, _)
{
    // Allow library declaration only as the first statement.
    if (!newFile)
    {
        ReportError("lib must be the first statement");
        return nullptr;
    }

    isLibrary = true;
    return nullptr;
}

DEFINE_ACTION
    (IncludeHeader, NonTerminal *, Token *, includeNameToken)
{
    newFile = false;

    // Switch to the new source file.
    currentSourceFileName = includeNameToken->s + ".asps";
    currentSourceLocation = includeNameToken->sourceLocation;

    delete includeNameToken;

    return nullptr;
}

DEFINE_ACTION
    (ImportModule, NonTerminal *,
     Token *, moduleNameToken, Token *, asNameToken)
{
    newFile = false;

    if (CheckReservedNameError(*asNameToken))
        return nullptr;

    // Ensure the import name has not been previously associated with a
    // different module.
    auto findImportIter = imports.find(asNameToken->s);
    if (findImportIter != imports.end() &&
        moduleNameToken->s != findImportIter->second)
    {
        ostringstream oss;
        oss
            << "Import module '" << findImportIter->second
            << "' previously imported as '" << asNameToken->s
            << "'; cannot also import module '" << moduleNameToken->s
            << "' as '" << asNameToken->s << '\'';
        ReportError(oss.str(), *asNameToken);
        return nullptr;
    }

    // Replace any previous definition having the same name with this
    // latter one.
    auto findModuleIter = currentModuleDefinitions->find(asNameToken->s);
    if (findModuleIter != currentModuleDefinitions->end())
    {
        ostringstream oss;
        oss << asNameToken->s << " redefined";
        ReportWarning(oss.str(), *asNameToken);
        currentModuleDefinitions->erase(findModuleIter);
        imports.erase(asNameToken->s);
    }

    // Add the import definition.
    currentModuleDefinitions->emplace
        (asNameToken->s, new Import(*moduleNameToken));
    imports.emplace(asNameToken->s, moduleNameToken->s);
    checkValueComputed = false;

    // Set the required app spec formats to support app modules.
    if (compilerAppSpecVersion < 2u)
        compilerAppSpecVersion = 2u;
    if (engineAppSpecVersion < 1u)
        engineAppSpecVersion = 1u;

    // Check if the module has been imported elsewhere.
    auto findModuleDefinitionIter = definitions.find(moduleNameToken->s);
    if (findModuleDefinitionIter == definitions.end())
    {
        // Switch to the new source file.
        currentSourceFileName = moduleNameToken->s + ".asps";
        currentSourceLocation = moduleNameToken->sourceLocation;

        // Switch to the new module import.
        currentModuleName = moduleNameToken->s;
        currentModuleDefinitions = &definitions.emplace
            (currentModuleName, map<string, shared_ptr<NonTerminal> >())
            .first->second;
    }

    currentSourceLocation = asNameToken->sourceLocation;

    if (asNameToken != moduleNameToken)
        delete asNameToken;
    delete moduleNameToken;

    return nullptr;
}

DEFINE_ACTION
    (MakeAssignment, NonTerminal *,
     Token *, nameToken, Literal *, value)
{
    newFile = false;

    if (CheckReservedNameError(*nameToken))
        return nullptr;

    // Replace any previous definition having the same name with this
    // latter one.
    auto findIter = currentModuleDefinitions->find(nameToken->s);
    if (findIter != currentModuleDefinitions->end())
    {
        ostringstream oss;
        oss << nameToken->s << " redefined";
        ReportWarning(oss.str(), *nameToken);
        currentModuleDefinitions->erase(findIter);
    }

    // Add the assignment definition.
    currentModuleDefinitions->emplace
        (nameToken->s, new Assignment(*nameToken, value));
    checkValueComputed = false;

    currentSourceLocation = nameToken->sourceLocation;

    delete nameToken;

    return nullptr;
}

DEFINE_ACTION
    (MakeFunction, NonTerminal *,
     Token *, nameToken, ParameterList *, parameterList,
     Token *, internalNameToken)
{
    newFile = false;

    if (CheckReservedNameError(*nameToken))
        return nullptr;

    // Ensure the validity of the order of parameter types.
    int position = 1;
    ValidFunctionDefinition validFunctionDefinition;
    for (auto iter = parameterList->ParametersBegin();
         validFunctionDefinition.IsValid() &&
         iter != parameterList->ParametersEnd();
         iter++, position++)
    {
        const auto &parameter = **iter;

        ValidFunctionDefinition::ParameterType type;
        bool typeValid = true;
        switch (parameter.GetType())
        {
            default:
                typeValid = false;
                break;
            case Parameter::Type::Positional:
                type = ValidFunctionDefinition::ParameterType::Positional;
                break;
            case Parameter::Type::TupleGroup:
                type = ValidFunctionDefinition::ParameterType::TupleGroup;
                break;
            case Parameter::Type::DictionaryGroup:
                type = ValidFunctionDefinition::ParameterType::DictionaryGroup;
                break;
        }
        if (!typeValid)
        {
            ReportError("Internal error; unknown parameter type", parameter);
            break;
        }

        string error = validFunctionDefinition.AddParameter
            (parameter.Name(), type, parameter.DefaultValue() != nullptr);
        if (!error.empty())
            ReportError(error, parameter);
    }

    auto parameterCount = parameterList->ParametersSize();
    if (parameterCount > AppSpecPrefix_MaxFunctionParameterCount &&
        engineAppSpecVersion < 1u)
        engineAppSpecVersion = 1u;

    // Replace any previous definition having the same name with this
    // latter one.
    auto findIter = currentModuleDefinitions->find(nameToken->s);
    if (findIter != currentModuleDefinitions->end())
    {
        ostringstream oss;
        oss << nameToken->s << " redefined";
        ReportWarning(oss.str(), *nameToken);
        currentModuleDefinitions->erase(findIter);
    }

    // Add the function definition.
    currentModuleDefinitions->emplace
        (nameToken->s, new FunctionDefinition
            (*nameToken, isLibrary,
             *internalNameToken, parameterList));
    checkValueComputed = false;

    currentSourceLocation = nameToken->sourceLocation;

    delete nameToken;
    delete internalNameToken;

    return nullptr;
}

DEFINE_ACTION
    (DeleteDefinition, NonTerminal *, NameList *, nameList)
{
    for (auto iter = nameList->NamesBegin();
         iter != nameList->NamesEnd();
         iter++)
    {
        const auto &name = *iter;

        auto findIter = currentModuleDefinitions->find(name);
        if (findIter == currentModuleDefinitions->end())
        {
            ostringstream oss;
            oss << "Cannot delete '" << name << '\'' << "; not found";
            ReportError(oss.str(), *nameList);
            continue;
        }

        currentModuleDefinitions->erase(findIter);
    }

    return nullptr;
}

DEFINE_ACTION
    (MakeEmptyParameterList, ParameterList *, int, _)
{
    return new ParameterList;
}

DEFINE_ACTION
    (AddParameterToList, ParameterList *,
     ParameterList *, parameterList, Parameter *, parameter)
{
    parameterList->Add(parameter);
    return parameterList;
}

DEFINE_ACTION
    (MakeParameter, Parameter *, Token *, nameToken)
{
    auto result = new Parameter(*nameToken);
    delete nameToken;
    return result;
}

DEFINE_ACTION
    (MakeDefaultedParameter, Parameter *,
     Token *, nameToken, Literal *, defaultValue)
{
    auto result = new Parameter(*nameToken, defaultValue);
    delete nameToken;
    return result;
}

DEFINE_ACTION
    (MakeTupleGroupParameter, Parameter *, Token *, nameToken)
{
    auto result = new Parameter(*nameToken, Parameter::Type::TupleGroup);
    delete nameToken;
    return result;
}

DEFINE_ACTION
    (MakeDictionaryGroupParameter, Parameter *, Token *, nameToken)
{
    auto result = new Parameter(*nameToken, Parameter::Type::DictionaryGroup);
    delete nameToken;
    return result;
}

DEFINE_ACTION
    (MakeEmptyNameList, NameList *, int, _)
{
    return new NameList;
}

DEFINE_ACTION
    (AddNameToList, NameList *, NameList *, nameList, Token *, nameToken)
{
    nameList->Add(*nameToken);
    delete nameToken;
    return nameList;
}

DEFINE_ACTION
    (MakeLiteral, Literal *, Token *, token)
{
    try
    {
        auto result = new Literal(*token);
        delete token;
        return result;
    }
    catch (const pair<SourceElement, string> &e)
    {
        ReportError(e.second, e.first);
    }
    catch (const string &e)
    {
        ReportError(e, *token);
    }

    return nullptr;
}

DEFINE_UTIL(FreeNonTerminal, void, NonTerminal *, nt)
{
    delete nt;
}

DEFINE_UTIL(FreeToken, void, Token *, token)
{
    delete token;
}

DEFINE_UTIL(ReportWarning, void, const char *, error)
{
     ReportWarning(string(error));
}

DEFINE_UTIL(ReportError, void, const char *, error)
{
     ReportError(string(error));
}

bool Generator::CheckReservedNameError(const Token &nameToken)
{
    if (AspIsNameReserved(nameToken.s.c_str()))
    {
        ostringstream oss;
        oss << "Cannot redefine reserved name '" << nameToken.s << '\'';
        ReportError(oss.str(), nameToken);
        return true;
    }

    return false;
}

void Generator::ReportWarning(const string &message) const
{
    ReportWarning(message, currentSourceLocation);
}

void Generator::ReportWarning
    (const string &message, const SourceElement &sourceElement) const
{
    ReportWarning(message, sourceElement.sourceLocation);
}

void Generator::ReportWarning
    (const string &message, const SourceLocation &sourceLocation) const
{
    ReportMessage(message, sourceLocation, false);
}

void Generator::ReportError(const string &message)
{
    ReportError(message, currentSourceLocation);
}

void Generator::ReportError
    (const string &message, const SourceElement &sourceElement)
{
    ReportError(message, sourceElement.sourceLocation);
}

void Generator::ReportError
    (const string &message, const SourceLocation &sourceLocation)
{
    ReportMessage(message, sourceLocation, true);
    errorCount++;
}

void Generator::ReportMessage
    (const string &message, const SourceLocation &sourceLocation, bool error)
    const
{
    if (sourceLocation.Defined())
        errorStream
            << sourceLocation.fileName << ':'
            << sourceLocation.line << ':'
            << sourceLocation.column << ": ";
    errorStream
        << (error ? "Error" : "Warning") << ": " << message << endl;
}
