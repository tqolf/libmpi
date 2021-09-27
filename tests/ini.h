#pragma once

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <cassert>

namespace config
{
enum {
    SI_OK = 0,       //!< No error
    SI_UPDATED = 1,  //!< An existing value was updated
    SI_INSERTED = 2, //!< A new value was inserted

    // note: test for any error with (retval < 0)
    SI_FAIL = -1,  //!< Generic failure
    SI_NOMEM = -2, //!< Out of memory error
    SI_FILE = -3   //!< File error (see errno for detail error)
};

#define SI_UTF8_SIGNATURE "\xEF\xBB\xBF"
#define SI_NEWLINE        "\n"

template <class Character, class Compare, class Encoder>
class IniTempl {
  public:
    struct Entry {
        const Character *item;
        const Character *comment;
        int order;

        Entry(const Character *item = NULL, int order = 0) : item(item), comment(NULL), order(order) {}
        Entry(const Character *item, const Character *comment, int order) : item(item), comment(comment), order(order)
        {
        }
        Entry(const Entry &rhs)
        {
            operator=(rhs);
        }
        Entry &operator=(const Entry &rhs)
        {
            item = rhs.item;
            comment = rhs.comment;
            order = rhs.order;
            return *this;
        }

        /** Strict less ordering by name of key only */
        struct KeyOrder {
            bool operator()(const Entry &lhs, const Entry &rhs) const
            {
                const static Compare isLess = Compare();
                return isLess(lhs.item, rhs.item);
            }
        };

        /** Strict less ordering by order, and then name of key */
        struct LoadOrder {
            bool operator()(const Entry &lhs, const Entry &rhs) const
            {
                if (lhs.order != rhs.order) {
                    return lhs.order < rhs.order;
                } else {
                    return KeyOrder()(lhs.item, rhs.item);
                }
            }
        };
    };

    /** map keys to values */
    typedef std::multimap<Entry, const Character *, typename Entry::KeyOrder> Pairs;

    /** map sections to key/value map */
    typedef std::map<Entry, Pairs, typename Entry::KeyOrder> Sections;

    /** set of dependent string pointers. Note that these pointers are
        dependent on memory owned by CSimpleIni.
    */
    typedef std::list<Entry> Entries;

    /** interface definition for the Writer */
    class Writer {
      public:
        Writer() {}
        virtual ~Writer() {}
        virtual void Write(const char *) = 0;

      private:
        Writer(const Writer &);            // disable
        Writer &operator=(const Writer &); // disable
    };

    /** Writer class to write the INI data to a file */
    class FileWriter : public Writer {
        FILE *m_file;

      public:
        FileWriter(FILE *a_file) : m_file(a_file) {}
        void Write(const char *a_pBuf)
        {
            fputs(a_pBuf, m_file);
        }

      private:
        FileWriter(const FileWriter &);            // disable
        FileWriter &operator=(const FileWriter &); // disable
    };

    /** Writer class to write the INI data to a string */
    class StringWriter : public Writer {
        std::string &m_string;

      public:
        StringWriter(std::string &a_string) : m_string(a_string) {}
        void Write(const char *a_pBuf)
        {
            m_string.append(a_pBuf);
        }

      private:
        StringWriter(const StringWriter &);            // disable
        StringWriter &operator=(const StringWriter &); // disable
    };

    /** Characterset conversion utility class to convert strings to the
        same format as is used for the storage.
    */
    class Converter : private Encoder {
      public:
        Converter(bool use_utf8) : Encoder(use_utf8)
        {
            scratch.resize(1024);
        }
        Converter(const Converter &rhs)
        {
            operator=(rhs);
        }
        Converter &operator=(const Converter &rhs)
        {
            scratch = rhs.scratch;
            return *this;
        }
        bool ConvertToStore(const Character *a_pszString)
        {
            size_t uLen = Encoder::SizeToStore(a_pszString);
            if (uLen == (size_t)(-1)) { return false; }
            while (uLen > scratch.size()) { scratch.resize(scratch.size() * 2); }
            return Encoder::ConvertToStore(a_pszString, const_cast<char *>(scratch.data()), scratch.size());
        }
        const char *Data()
        {
            return scratch.data();
        }

      private:
        std::string scratch;
    };

  public:
    IniTempl(bool use_utf8 = false, bool allow_multikey = false, bool support_multiline = false);
    ~IniTempl();

    /** Deallocate all memory stored by this object */
    void Reset();

    /** Has any data been loaded */
    bool IsEmpty() const
    {
        return m_data.empty();
    }

    void SetUnicode(bool use_utf8 = true)
    {
        if (!m_pData) m_bStoreIsUtf8 = use_utf8;
    }

    /** Get the storage format of the INI data. */
    bool IsUnicode() const
    {
        return m_bStoreIsUtf8;
    }

    void SetMultiKey(bool a_bAllowMultiKey = true)
    {
        m_bAllowMultiKey = a_bAllowMultiKey;
    }

    /** Get the storage format of the INI data. */
    bool IsMultiKey() const
    {
        return m_bAllowMultiKey;
    }

    void SetMultiLine(bool a_bAllowMultiLine = true)
    {
        m_bAllowMultiLine = a_bAllowMultiLine;
    }

    /** Query the status of multi-line data */
    bool IsMultiLine() const
    {
        return m_bAllowMultiLine;
    }

    void SetSpaces(bool a_bSpaces = true)
    {
        m_bSpaces = a_bSpaces;
    }

    /** Query the status of spaces output */
    bool UsingSpaces() const
    {
        return m_bSpaces;
    }

    int LoadFile(const char *a_pszFile);

    int LoadFile(FILE *a_fpFile);

    int LoadData(const std::string &a_strData)
    {
        return LoadData(a_strData.c_str(), a_strData.size());
    }

    int LoadData(const char *a_pData, size_t a_uDataLen);

    int SaveFile(const char *a_pszFile, bool a_bAddSignature = true) const;
    int SaveFile(FILE *a_pFile, bool a_bAddSignature = false) const;
    int Save(Writer &a_oOutput, bool a_bAddSignature = false) const;
    int Save(std::string &a_sBuffer, bool a_bAddSignature = false) const
    {
        StringWriter writer(a_sBuffer);
        return Save(writer, a_bAddSignature);
    }

    void GetAllSections(Entries &a_names) const;
    bool GetAllKeys(const Character *a_pSection, Entries &a_names) const;
    bool GetAllValues(const Character *a_pSection, const Character *a_pKey, Entries &a_values) const;
    int GetSectionSize(const Character *a_pSection) const;
    const Pairs *GetSection(const Character *a_pSection) const;

    const Character *GetValue(const Character *a_pSection, const Character *a_pKey, const Character *a_pDefault = NULL,
                              bool *a_pHasMultiple = NULL) const;
    long GetLongValue(const Character *a_pSection, const Character *a_pKey, long a_nDefault = 0,
                      bool *a_pHasMultiple = NULL) const;
    double GetDoubleValue(const Character *a_pSection, const Character *a_pKey, double a_nDefault = 0,
                          bool *a_pHasMultiple = NULL) const;
    bool GetBoolValue(const Character *a_pSection, const Character *a_pKey, bool a_bDefault = false,
                      bool *a_pHasMultiple = NULL) const;

    int SetValue(const Character *a_pSection, const Character *a_pKey, const Character *a_pValue,
                 const Character *a_pComment = NULL, bool a_bForceReplace = false)
    {
        return AddEntry(a_pSection, a_pKey, a_pValue, a_pComment, a_bForceReplace, true);
    }

    int SetLongValue(const Character *a_pSection, const Character *a_pKey, long a_nValue,
                     const Character *a_pComment = NULL, bool a_bUseHex = false, bool a_bForceReplace = false);
    int SetDoubleValue(const Character *a_pSection, const Character *a_pKey, double a_nValue,
                       const Character *a_pComment = NULL, bool a_bForceReplace = false);

    int SetBoolValue(const Character *a_pSection, const Character *a_pKey, bool a_bValue,
                     const Character *a_pComment = NULL, bool a_bForceReplace = false);

    bool Delete(const Character *a_pSection, const Character *a_pKey, bool a_bRemoveEmpty = false);
    bool DeleteValue(const Character *a_pSection, const Character *a_pKey, const Character *a_pValue,
                     bool a_bRemoveEmpty = false);

    Converter GetConverter() const
    {
        return Converter(m_bStoreIsUtf8);
    }

  private:
    // copying is not permitted
    IniTempl(const IniTempl &);            // disabled
    IniTempl &operator=(const IniTempl &); // disabled

    int FindFileComment(Character *&a_pData, bool a_bCopyStrings);

    bool FindEntry(Character *&a_pData, const Character *&a_pSection, const Character *&a_pKey,
                   const Character *&a_pVal, const Character *&a_pComment) const;

    int AddEntry(const Character *a_pSection, const Character *a_pKey, const Character *a_pValue,
                 const Character *a_pComment, bool a_bForceReplace, bool a_bCopyStrings);

    /** Is the supplied character a whitespace character? */
    inline bool IsSpace(Character ch) const
    {
        return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
    }

    /** Does the supplied character start a comment line? */
    inline bool IsComment(Character ch) const
    {
        return (ch == ';' || ch == '#');
    }

    /** Skip over a newline character (or characters) for either DOS or UNIX */
    inline void SkipNewLine(Character *&a_pData) const
    {
        a_pData += (*a_pData == '\r' && *(a_pData + 1) == '\n') ? 2 : 1;
    }

    /** Make a copy of the supplied string, replacing the original pointer */
    int CopyString(const Character *&a_pString);

    /** Delete a string from the copied strings buffer if necessary */
    void DeleteString(const Character *a_pString);

    /** Internal use of our string comparison function */
    bool IsLess(const Character *a_pLeft, const Character *a_pRight) const
    {
        const static Compare isLess = Compare();
        return isLess(a_pLeft, a_pRight);
    }

    bool IsMultiLineTag(const Character *a_pData) const;
    bool IsMultiLineData(const Character *a_pData) const;
    bool LoadMultiLineText(Character *&a_pData, const Character *&a_pVal, const Character *a_pTagName,
                           bool a_bAllowBlankLinesInComment = false) const;
    bool IsNewLineChar(Character a_c) const;

    bool OutputMultiLineText(Writer &a_oOutput, Converter &a_oConverter, const Character *a_pText) const;

  private:
    /** Copy of the INI file data in our character format. This will be
        modified when parsed to have NULL characters added after all
        interesting string entries. All of the string pointers to sections,
        keys and values point into this block of memory.
     */
    Character *m_pData;

    /** Length of the data that we have stored. Used when deleting strings
        to determine if the string is stored here or in the allocated string
        buffer.
     */
    size_t m_uDataLen;

    /** File comment for this data, if one exists. */
    const Character *m_pFileComment;

    /** Parsed INI data. Section -> (Key -> Value). */
    Sections m_data;

    /** This vector stores allocated memory for copies of strings that have
        been supplied after the file load. It will be empty unless SetValue()
        has been called.
     */
    Entries m_strings;

    /** Is the format of our datafile UTF-8 or MBCS? */
    bool m_bStoreIsUtf8;

    /** Are multiple values permitted for the same key? */
    bool m_bAllowMultiKey;

    /** Are data values permitted to span multiple lines? */
    bool m_bAllowMultiLine;

    /** Should spaces be written out surrounding the equals sign? */
    bool m_bSpaces;

    /** Next order value, used to ensure sections and keys are output in the
        same order that they are loaded/added.
     */
    int m_nOrder;
};

template <class Character, class Compare, class Encoder>
IniTempl<Character, Compare, Encoder>::IniTempl(bool use_utf8, bool a_bAllowMultiKey, bool a_bAllowMultiLine)
    : m_pData(0),
      m_uDataLen(0),
      m_pFileComment(NULL),
      m_bStoreIsUtf8(use_utf8),
      m_bAllowMultiKey(a_bAllowMultiKey),
      m_bAllowMultiLine(a_bAllowMultiLine),
      m_bSpaces(true),
      m_nOrder(0)
{
}

template <class Character, class Compare, class Encoder>
IniTempl<Character, Compare, Encoder>::~IniTempl()
{
    Reset();
}

template <class Character, class Compare, class Encoder>
void IniTempl<Character, Compare, Encoder>::Reset()
{
    // remove all data
    delete[] m_pData;
    m_pData = NULL;
    m_uDataLen = 0;
    m_pFileComment = NULL;
    if (!m_data.empty()) { m_data.erase(m_data.begin(), m_data.end()); }

    // remove all strings
    if (!m_strings.empty()) {
        typename Entries::iterator i = m_strings.begin();
        for (; i != m_strings.end(); ++i) { delete[] const_cast<Character *>(i->item); }
        m_strings.erase(m_strings.begin(), m_strings.end());
    }
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::LoadFile(const char *a_pszFile)
{
    FILE *fp = NULL;
    fp = fopen(a_pszFile, "rb");
    if (!fp) { return SI_FILE; }
    int rc = LoadFile(fp);
    fclose(fp);
    return rc;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::LoadFile(FILE *a_fpFile)
{
    // load the raw file data
    int retval = fseek(a_fpFile, 0, SEEK_END);
    if (retval != 0) { return SI_FILE; }
    long lSize = ftell(a_fpFile);
    if (lSize < 0) { return SI_FILE; }
    if (lSize == 0) { return SI_OK; }

    // allocate and ensure NULL terminated
    char *pData = new (std::nothrow) char[lSize + 1];
    if (!pData) { return SI_NOMEM; }
    pData[lSize] = 0;

    // load data into buffer
    fseek(a_fpFile, 0, SEEK_SET);
    size_t uRead = fread(pData, sizeof(char), lSize, a_fpFile);
    if (uRead != (size_t)lSize) {
        delete[] pData;
        return SI_FILE;
    }

    // convert the raw data to unicode
    int rc = LoadData(pData, uRead);
    delete[] pData;
    return rc;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::LoadData(const char *a_pData, size_t a_uDataLen)
{
    if (!a_pData) { return SI_OK; }

    // if the UTF-8 BOM exists, consume it and set mode to unicode, if we have
    // already loaded data and try to change mode half-way through then this will
    // be ignored and we will assert in debug versions
    if (a_uDataLen >= 3 && memcmp(a_pData, SI_UTF8_SIGNATURE, 3) == 0) {
        a_pData += 3;
        a_uDataLen -= 3;
        assert(m_bStoreIsUtf8 || !m_pData); // we don't expect mixed mode data
        SetUnicode();
    }

    if (a_uDataLen == 0) { return SI_OK; }

    // determine the length of the converted data
    Encoder converter(m_bStoreIsUtf8);
    size_t uLen = converter.SizeFromStore(a_pData, a_uDataLen);
    if (uLen == (size_t)(-1)) { return SI_FAIL; }

    // allocate memory for the data, ensure that there is a NULL
    // terminator wherever the converted data ends
    Character *pData = new (std::nothrow) Character[uLen + 1];
    if (!pData) { return SI_NOMEM; }
    memset(pData, 0, sizeof(Character) * (uLen + 1));

    // convert the data
    if (!converter.ConvertFromStore(a_pData, a_uDataLen, pData, uLen)) {
        delete[] pData;
        return SI_FAIL;
    }

    // parse it
    const static Character empty = 0;
    Character *pWork = pData;
    const Character *pSection = &empty;
    const Character *item = NULL;
    const Character *pVal = NULL;
    const Character *comment = NULL;

    // We copy the strings if we are loading data into this class when we
    // already have stored some.
    bool bCopyStrings = (m_pData != NULL);

    // find a file comment if it exists, this is a comment that starts at the
    // beginning of the file and continues until the first blank line.
    int rc = FindFileComment(pWork, bCopyStrings);
    if (rc < 0) return rc;

    // add every entry in the file to the data table
    while (FindEntry(pWork, pSection, item, pVal, comment)) {
        rc = AddEntry(pSection, item, pVal, comment, false, bCopyStrings);
        if (rc < 0) return rc;
    }

    // store these strings if we didn't copy them
    if (bCopyStrings) {
        delete[] pData;
    } else {
        m_pData = pData;
        m_uDataLen = uLen + 1;
    }

    return SI_OK;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::FindFileComment(Character *&a_pData, bool a_bCopyStrings)
{
    // there can only be a single file comment
    if (m_pFileComment) { return SI_OK; }

    // Load the file comment as multi-line text, this will modify all of
    // the newline characters to be single \n chars
    if (!LoadMultiLineText(a_pData, m_pFileComment, NULL, false)) { return SI_OK; }

    // copy the string if necessary
    if (a_bCopyStrings) {
        int rc = CopyString(m_pFileComment);
        if (rc < 0) return rc;
    }

    return SI_OK;
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::FindEntry(Character *&a_pData, const Character *&a_pSection,
                                                      const Character *&a_pKey, const Character *&a_pVal,
                                                      const Character *&a_pComment) const
{
    a_pComment = NULL;

    Character *pTrail = NULL;
    while (*a_pData) {
        // skip spaces and empty lines
        while (*a_pData && IsSpace(*a_pData)) { ++a_pData; }
        if (!*a_pData) { break; }

        // skip processing of comment lines but keep a pointer to
        // the start of the comment.
        if (IsComment(*a_pData)) {
            LoadMultiLineText(a_pData, a_pComment, NULL, true);
            continue;
        }

        // process section names
        if (*a_pData == '[') {
            // skip leading spaces
            ++a_pData;
            while (*a_pData && IsSpace(*a_pData)) { ++a_pData; }

            // find the end of the section name (it may contain spaces)
            // and convert it to lowercase as necessary
            a_pSection = a_pData;
            while (*a_pData && *a_pData != ']' && !IsNewLineChar(*a_pData)) { ++a_pData; }

            // if it's an invalid line, just skip it
            if (*a_pData != ']') { continue; }

            // remove trailing spaces from the section
            pTrail = a_pData - 1;
            while (pTrail >= a_pSection && IsSpace(*pTrail)) { --pTrail; }
            ++pTrail;
            *pTrail = 0;

            // skip to the end of the line
            ++a_pData; // safe as checked that it == ']' above
            while (*a_pData && !IsNewLineChar(*a_pData)) { ++a_pData; }

            a_pKey = NULL;
            a_pVal = NULL;
            return true;
        }

        // find the end of the key name (it may contain spaces)
        // and convert it to lowercase as necessary
        a_pKey = a_pData;
        while (*a_pData && *a_pData != '=' && !IsNewLineChar(*a_pData)) { ++a_pData; }

        // if it's an invalid line, just skip it
        if (*a_pData != '=') { continue; }

        // empty keys are invalid
        if (a_pKey == a_pData) {
            while (*a_pData && !IsNewLineChar(*a_pData)) { ++a_pData; }
            continue;
        }

        // remove trailing spaces from the key
        pTrail = a_pData - 1;
        while (pTrail >= a_pKey && IsSpace(*pTrail)) { --pTrail; }
        ++pTrail;
        *pTrail = 0;

        // skip leading whitespace on the value
        ++a_pData; // safe as checked that it == '=' above
        while (*a_pData && !IsNewLineChar(*a_pData) && IsSpace(*a_pData)) { ++a_pData; }

        // find the end of the value which is the end of this line
        a_pVal = a_pData;
        while (*a_pData && !IsNewLineChar(*a_pData)) { ++a_pData; }

        // remove trailing spaces from the value
        pTrail = a_pData - 1;
        if (*a_pData) { // prepare for the next round
            SkipNewLine(a_pData);
        }
        while (pTrail >= a_pVal && IsSpace(*pTrail)) { --pTrail; }
        ++pTrail;
        *pTrail = 0;

        // check for multi-line entries
        if (m_bAllowMultiLine && IsMultiLineTag(a_pVal)) {
            // skip the "<<<" to get the tag that will end the multiline
            const Character *pTagName = a_pVal + 3;
            return LoadMultiLineText(a_pData, a_pVal, pTagName);
        }

        // return the standard entry
        return true;
    }

    return false;
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::IsMultiLineTag(const Character *a_pVal) const
{
    // check for the "<<<" prefix for a multi-line entry
    if (*a_pVal++ != '<') return false;
    if (*a_pVal++ != '<') return false;
    if (*a_pVal++ != '<') return false;
    return true;
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::IsMultiLineData(const Character *a_pData) const
{
    // data is multi-line if it has any of the following features:
    //  * whitespace prefix
    //  * embedded newlines
    //  * whitespace suffix

    // empty string
    if (!*a_pData) { return false; }

    // check for prefix
    if (IsSpace(*a_pData)) { return true; }

    // embedded newlines
    while (*a_pData) {
        if (IsNewLineChar(*a_pData)) { return true; }
        ++a_pData;
    }

    // check for suffix
    if (IsSpace(*--a_pData)) { return true; }

    return false;
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::IsNewLineChar(Character a_c) const
{
    return (a_c == '\n' || a_c == '\r');
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::LoadMultiLineText(Character *&a_pData, const Character *&a_pVal,
                                                              const Character *a_pTagName,
                                                              bool a_bAllowBlankLinesInComment) const
{
    // we modify this data to strip all newlines down to a single '\n'
    // character. This means that on Windows we need to strip out some
    // characters which will make the data shorter.
    // i.e.  LINE1-LINE1\r\nLINE2-LINE2\0 will become
    //       LINE1-LINE1\nLINE2-LINE2\0
    // The pDataLine entry is the pointer to the location in memory that
    // the current line needs to start to run following the existing one.
    // This may be the same as pCurrLine in which case no move is needed.
    Character *pDataLine = a_pData;
    Character *pCurrLine;

    // value starts at the current line
    a_pVal = a_pData;

    // find the end tag. This tag must start in column 1 and be
    // followed by a newline. We ignore any whitespace after the end
    // tag but not whitespace before it.
    Character cEndOfLineChar = *a_pData;
    for (;;) {
        // if we are loading comments then we need a comment character as
        // the first character on every line
        if (!a_pTagName && !IsComment(*a_pData)) {
            // if we aren't allowing blank lines then we're done
            if (!a_bAllowBlankLinesInComment) { break; }

            // if we are allowing blank lines then we only include them
            // in this comment if another comment follows, so read ahead
            // to find out.
            Character *pCurr = a_pData;
            int nNewLines = 0;
            while (IsSpace(*pCurr)) {
                if (IsNewLineChar(*pCurr)) {
                    ++nNewLines;
                    SkipNewLine(pCurr);
                } else {
                    ++pCurr;
                }
            }

            // we have a comment, add the blank lines to the output
            // and continue processing from here
            if (IsComment(*pCurr)) {
                for (; nNewLines > 0; --nNewLines) *pDataLine++ = '\n';
                a_pData = pCurr;
                continue;
            }

            // the comment ends here
            break;
        }

        // find the end of this line
        pCurrLine = a_pData;
        while (*a_pData && !IsNewLineChar(*a_pData)) ++a_pData;

        // move this line down to the location that it should be if necessary
        if (pDataLine < pCurrLine) {
            size_t nLen = (size_t)(a_pData - pCurrLine);
            memmove(pDataLine, pCurrLine, nLen * sizeof(Character));
            pDataLine[nLen] = '\0';
        }

        // end the line with a NULL
        cEndOfLineChar = *a_pData;
        *a_pData = 0;

        // if are looking for a tag then do the check now. This is done before
        // checking for end of the data, so that if we have the tag at the end
        // of the data then the tag is removed correctly.
        if (a_pTagName) {
            // strip whitespace from the end of this tag
            Character *pc = a_pData - 1;
            while (pc > pDataLine && IsSpace(*pc)) --pc;
            Character ch = *++pc;
            *pc = 0;

            if (!IsLess(pDataLine, a_pTagName) && !IsLess(a_pTagName, pDataLine)) { break; }

            *pc = ch;
        }

        // if we are at the end of the data then we just automatically end
        // this entry and return the current data.
        if (!cEndOfLineChar) { return true; }

        // otherwise we need to process this newline to ensure that it consists
        // of just a single \n character.
        pDataLine += (a_pData - pCurrLine);
        *a_pData = cEndOfLineChar;
        SkipNewLine(a_pData);
        *pDataLine++ = '\n';
    }

    // if we didn't find a comment at all then return false
    if (a_pVal == a_pData) {
        a_pVal = NULL;
        return false;
    }

    // the data (which ends at the end of the last line) needs to be
    // null-terminated BEFORE before the newline character(s). If the
    // user wants a new line in the multi-line data then they need to
    // add an empty line before the tag.
    *--pDataLine = '\0';

    // if looking for a tag and if we aren't at the end of the data,
    // then move a_pData to the start of the next line.
    if (a_pTagName && cEndOfLineChar) {
        assert(IsNewLineChar(cEndOfLineChar));
        *a_pData = cEndOfLineChar;
        SkipNewLine(a_pData);
    }

    return true;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::CopyString(const Character *&a_pString)
{
    size_t uLen = 0;
    if (sizeof(Character) == sizeof(char)) {
        uLen = strlen((const char *)a_pString);
    } else if (sizeof(Character) == sizeof(wchar_t)) {
        uLen = wcslen((const wchar_t *)a_pString);
    } else {
        for (; a_pString[uLen]; ++uLen) /*loop*/
            ;
    }
    ++uLen; // NULL character
    Character *pCopy = new (std::nothrow) Character[uLen];
    if (!pCopy) { return SI_NOMEM; }
    memcpy(pCopy, a_pString, sizeof(Character) * uLen);
    m_strings.push_back(pCopy);
    a_pString = pCopy;
    return SI_OK;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::AddEntry(const Character *a_pSection, const Character *a_pKey,
                                                    const Character *a_pValue, const Character *a_pComment,
                                                    bool a_bForceReplace, bool a_bCopyStrings)
{
    int rc;
    bool bInserted = false;

    assert(!a_pComment || IsComment(*a_pComment));

    // if we are copying strings then make a copy of the comment now
    // because we will need it when we add the entry.
    if (a_bCopyStrings && a_pComment) {
        rc = CopyString(a_pComment);
        if (rc < 0) return rc;
    }

    // create the section entry if necessary
    typename Sections::iterator iSection = m_data.find(a_pSection);
    if (iSection == m_data.end()) {
        // if the section doesn't exist then we need a copy as the
        // string needs to last beyond the end of this function
        if (a_bCopyStrings) {
            rc = CopyString(a_pSection);
            if (rc < 0) return rc;
        }

        // only set the comment if this is a section only entry
        Entry oSection(a_pSection, ++m_nOrder);
        if (a_pComment && (!a_pKey || !a_pValue)) { oSection.comment = a_pComment; }

        typename Sections::value_type oEntry(oSection, Pairs());
        typedef typename Sections::iterator SectionIterator;
        std::pair<SectionIterator, bool> i = m_data.insert(oEntry);
        iSection = i.first;
        bInserted = true;
    }
    if (!a_pKey || !a_pValue) {
        // section only entries are specified with item and pVal as NULL
        return bInserted ? SI_INSERTED : SI_UPDATED;
    }

    // check for existence of the key
    Pairs &keyval = iSection->second;
    typename Pairs::iterator iKey = keyval.find(a_pKey);
    bInserted = iKey == keyval.end();

    // remove all existing entries but save the load order and
    // comment of the first entry
    int nLoadOrder = ++m_nOrder;
    if (iKey != keyval.end() && m_bAllowMultiKey && a_bForceReplace) {
        const Character *comment = NULL;
        while (iKey != keyval.end() && !IsLess(a_pKey, iKey->first.item)) {
            if (iKey->first.order < nLoadOrder) {
                nLoadOrder = iKey->first.order;
                comment = iKey->first.comment;
            }
            ++iKey;
        }
        if (comment) {
            DeleteString(a_pComment);
            a_pComment = comment;
            CopyString(a_pComment);
        }
        Delete(a_pSection, a_pKey);
        iKey = keyval.end();
    }

    // make string copies if necessary
    bool bForceCreateNewKey = m_bAllowMultiKey && !a_bForceReplace;
    if (a_bCopyStrings) {
        if (bForceCreateNewKey || iKey == keyval.end()) {
            // if the key doesn't exist then we need a copy as the
            // string needs to last beyond the end of this function
            // because we will be inserting the key next
            rc = CopyString(a_pKey);
            if (rc < 0) return rc;
        }

        // we always need a copy of the value
        rc = CopyString(a_pValue);
        if (rc < 0) return rc;
    }

    // create the key entry
    if (iKey == keyval.end() || bForceCreateNewKey) {
        Entry oKey(a_pKey, nLoadOrder);
        if (a_pComment) { oKey.comment = a_pComment; }
        typename Pairs::value_type oEntry(oKey, static_cast<const Character *>(NULL));
        iKey = keyval.insert(oEntry);
    }

    iKey->second = a_pValue;
    return bInserted ? SI_INSERTED : SI_UPDATED;
}

template <class Character, class Compare, class Encoder>
const Character *IniTempl<Character, Compare, Encoder>::GetValue(const Character *a_pSection, const Character *a_pKey,
                                                                 const Character *a_pDefault,
                                                                 bool *a_pHasMultiple) const
{
    if (a_pHasMultiple) { *a_pHasMultiple = false; }
    if (!a_pSection || !a_pKey) { return a_pDefault; }
    typename Sections::const_iterator iSection = m_data.find(a_pSection);
    if (iSection == m_data.end()) { return a_pDefault; }
    typename Pairs::const_iterator iKeyVal = iSection->second.find(a_pKey);
    if (iKeyVal == iSection->second.end()) { return a_pDefault; }

    // check for multiple entries with the same key
    if (m_bAllowMultiKey && a_pHasMultiple) {
        typename Pairs::const_iterator iTemp = iKeyVal;
        if (++iTemp != iSection->second.end()) {
            if (!IsLess(a_pKey, iTemp->first.item)) { *a_pHasMultiple = true; }
        }
    }

    return iKeyVal->second;
}

template <class Character, class Compare, class Encoder>
long IniTempl<Character, Compare, Encoder>::GetLongValue(const Character *a_pSection, const Character *a_pKey,
                                                         long a_nDefault, bool *a_pHasMultiple) const
{
    // return the default if we don't have a value
    const Character *pszValue = GetValue(a_pSection, a_pKey, NULL, a_pHasMultiple);
    if (!pszValue || !*pszValue) return a_nDefault;

    // convert to UTF-8/MBCS which for a numeric value will be the same as ASCII
    char szValue[64] = {0};
    Encoder c(m_bStoreIsUtf8);
    if (!c.ConvertToStore(pszValue, szValue, sizeof(szValue))) { return a_nDefault; }

    // handle the value as hex if prefaced with "0x"
    long nValue = a_nDefault;
    char *pszSuffix = szValue;
    if (szValue[0] == '0' && (szValue[1] == 'x' || szValue[1] == 'X')) {
        if (!szValue[2]) return a_nDefault;
        nValue = strtol(&szValue[2], &pszSuffix, 16);
    } else {
        nValue = strtol(szValue, &pszSuffix, 10);
    }

    // any invalid strings will return the default value
    if (*pszSuffix) { return a_nDefault; }

    return nValue;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::SetLongValue(const Character *a_pSection, const Character *a_pKey,
                                                        long a_nValue, const Character *a_pComment, bool a_bUseHex,
                                                        bool a_bForceReplace)
{
    // use SetValue to create sections
    if (!a_pSection || !a_pKey) return SI_FAIL;

    // convert to an ASCII string
    char szInput[64];
    sprintf(szInput, a_bUseHex ? "0x%lx" : "%ld", a_nValue);

    // convert to output text
    Character szOutput[64];
    Encoder c(m_bStoreIsUtf8);
    c.ConvertFromStore(szInput, strlen(szInput) + 1, szOutput, sizeof(szOutput) / sizeof(Character));

    // actually add it
    return AddEntry(a_pSection, a_pKey, szOutput, a_pComment, a_bForceReplace, true);
}

template <class Character, class Compare, class Encoder>
double IniTempl<Character, Compare, Encoder>::GetDoubleValue(const Character *a_pSection, const Character *a_pKey,
                                                             double a_nDefault, bool *a_pHasMultiple) const
{
    // return the default if we don't have a value
    const Character *pszValue = GetValue(a_pSection, a_pKey, NULL, a_pHasMultiple);
    if (!pszValue || !*pszValue) return a_nDefault;

    // convert to UTF-8/MBCS which for a numeric value will be the same as ASCII
    char szValue[64] = {0};
    Encoder c(m_bStoreIsUtf8);
    if (!c.ConvertToStore(pszValue, szValue, sizeof(szValue))) { return a_nDefault; }

    char *pszSuffix = NULL;
    double nValue = strtod(szValue, &pszSuffix);

    // any invalid strings will return the default value
    if (!pszSuffix || *pszSuffix) { return a_nDefault; }

    return nValue;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::SetDoubleValue(const Character *a_pSection, const Character *a_pKey,
                                                          double a_nValue, const Character *a_pComment,
                                                          bool a_bForceReplace)
{
    // use SetValue to create sections
    if (!a_pSection || !a_pKey) return SI_FAIL;

    // convert to an ASCII string
    char szInput[64];
    sprintf(szInput, "%f", a_nValue);

    // convert to output text
    Character szOutput[64];
    Encoder c(m_bStoreIsUtf8);
    c.ConvertFromStore(szInput, strlen(szInput) + 1, szOutput, sizeof(szOutput) / sizeof(Character));

    // actually add it
    return AddEntry(a_pSection, a_pKey, szOutput, a_pComment, a_bForceReplace, true);
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::GetBoolValue(const Character *a_pSection, const Character *a_pKey,
                                                         bool a_bDefault, bool *a_pHasMultiple) const
{
    // return the default if we don't have a value
    const Character *pszValue = GetValue(a_pSection, a_pKey, NULL, a_pHasMultiple);
    if (!pszValue || !*pszValue) return a_bDefault;

    // we only look at the minimum number of characters
    switch (pszValue[0]) {
        case 't':
        case 'T': // true
        case 'y':
        case 'Y': // yes
        case '1': // 1 (one)
            return true;

        case 'f':
        case 'F': // false
        case 'n':
        case 'N': // no
        case '0': // 0 (zero)
            return false;

        case 'o':
        case 'O':
            if (pszValue[1] == 'n' || pszValue[1] == 'N') return true;  // on
            if (pszValue[1] == 'f' || pszValue[1] == 'F') return false; // off
            break;
    }

    // no recognized value, return the default
    return a_bDefault;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::SetBoolValue(const Character *a_pSection, const Character *a_pKey,
                                                        bool a_bValue, const Character *a_pComment,
                                                        bool a_bForceReplace)
{
    // use SetValue to create sections
    if (!a_pSection || !a_pKey) return SI_FAIL;

    // convert to an ASCII string
    const char *pszInput = a_bValue ? "true" : "false";

    // convert to output text
    Character szOutput[64];
    Encoder c(m_bStoreIsUtf8);
    c.ConvertFromStore(pszInput, strlen(pszInput) + 1, szOutput, sizeof(szOutput) / sizeof(Character));

    // actually add it
    return AddEntry(a_pSection, a_pKey, szOutput, a_pComment, a_bForceReplace, true);
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::GetAllValues(const Character *a_pSection, const Character *a_pKey,
                                                         Entries &a_values) const
{
    a_values.clear();

    if (!a_pSection || !a_pKey) { return false; }
    typename Sections::const_iterator iSection = m_data.find(a_pSection);
    if (iSection == m_data.end()) { return false; }
    typename Pairs::const_iterator iKeyVal = iSection->second.find(a_pKey);
    if (iKeyVal == iSection->second.end()) { return false; }

    // insert all values for this key
    a_values.push_back(Entry(iKeyVal->second, iKeyVal->first.comment, iKeyVal->first.order));
    if (m_bAllowMultiKey) {
        ++iKeyVal;
        while (iKeyVal != iSection->second.end() && !IsLess(a_pKey, iKeyVal->first.item)) {
            a_values.push_back(Entry(iKeyVal->second, iKeyVal->first.comment, iKeyVal->first.order));
            ++iKeyVal;
        }
    }

    return true;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::GetSectionSize(const Character *a_pSection) const
{
    if (!a_pSection) { return -1; }

    typename Sections::const_iterator iSection = m_data.find(a_pSection);
    if (iSection == m_data.end()) { return -1; }
    const Pairs &section = iSection->second;

    // if multi-key isn't permitted then the section size is
    // the number of keys that we have.
    if (!m_bAllowMultiKey || section.empty()) { return (int)section.size(); }

    // otherwise we need to count them
    int nCount = 0;
    const Character *pLastKey = NULL;
    typename Pairs::const_iterator iKeyVal = section.begin();
    for (int n = 0; iKeyVal != section.end(); ++iKeyVal, ++n) {
        if (!pLastKey || IsLess(pLastKey, iKeyVal->first.item)) {
            ++nCount;
            pLastKey = iKeyVal->first.item;
        }
    }
    return nCount;
}

template <class Character, class Compare, class Encoder>
const typename IniTempl<Character, Compare, Encoder>::Pairs *IniTempl<Character, Compare, Encoder>::GetSection(
    const Character *a_pSection) const
{
    if (a_pSection) {
        typename Sections::const_iterator i = m_data.find(a_pSection);
        if (i != m_data.end()) { return &(i->second); }
    }
    return 0;
}

template <class Character, class Compare, class Encoder>
void IniTempl<Character, Compare, Encoder>::GetAllSections(Entries &a_names) const
{
    a_names.clear();
    typename Sections::const_iterator i = m_data.begin();
    for (int n = 0; i != m_data.end(); ++i, ++n) { a_names.push_back(i->first); }
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::GetAllKeys(const Character *a_pSection, Entries &a_names) const
{
    a_names.clear();

    if (!a_pSection) { return false; }

    typename Sections::const_iterator iSection = m_data.find(a_pSection);
    if (iSection == m_data.end()) { return false; }

    const Pairs &section = iSection->second;
    const Character *pLastKey = NULL;
    typename Pairs::const_iterator iKeyVal = section.begin();
    for (int n = 0; iKeyVal != section.end(); ++iKeyVal, ++n) {
        if (!pLastKey || IsLess(pLastKey, iKeyVal->first.item)) {
            a_names.push_back(iKeyVal->first);
            pLastKey = iKeyVal->first.item;
        }
    }

    return true;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::SaveFile(const char *a_pszFile, bool a_bAddSignature) const
{
    FILE *fp = NULL;
    fp = fopen(a_pszFile, "wb");
    if (!fp) return SI_FILE;
    int rc = SaveFile(fp, a_bAddSignature);
    fclose(fp);
    return rc;
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::SaveFile(FILE *a_pFile, bool a_bAddSignature) const
{
    FileWriter writer(a_pFile);
    return Save(writer, a_bAddSignature);
}

template <class Character, class Compare, class Encoder>
int IniTempl<Character, Compare, Encoder>::Save(Writer &a_oOutput, bool a_bAddSignature) const
{
    Converter convert(m_bStoreIsUtf8);

    // add the UTF-8 signature if it is desired
    if (m_bStoreIsUtf8 && a_bAddSignature) { a_oOutput.Write(SI_UTF8_SIGNATURE); }

    // get all of the sections sorted in load order
    Entries oSections;
    GetAllSections(oSections);
    oSections.sort(typename Entry::LoadOrder());

    // if there is an empty section name, then it must be written out first
    // regardless of the load order
    typename Entries::iterator is = oSections.begin();
    for (; is != oSections.end(); ++is) {
        if (!*is->item) {
            // move the empty section name to the front of the section list
            if (is != oSections.begin()) { oSections.splice(oSections.begin(), oSections, is, std::next(is)); }
            break;
        }
    }

    // write the file comment if we have one
    bool bNeedNewLine = false;
    if (m_pFileComment) {
        if (!OutputMultiLineText(a_oOutput, convert, m_pFileComment)) { return SI_FAIL; }
        bNeedNewLine = true;
    }

    // iterate through our sections and output the data
    typename Entries::const_iterator iSection = oSections.begin();
    for (; iSection != oSections.end(); ++iSection) {
        // write out the comment if there is one
        if (iSection->comment) {
            if (bNeedNewLine) {
                a_oOutput.Write(SI_NEWLINE);
                a_oOutput.Write(SI_NEWLINE);
            }
            if (!OutputMultiLineText(a_oOutput, convert, iSection->comment)) { return SI_FAIL; }
            bNeedNewLine = false;
        }

        if (bNeedNewLine) {
            a_oOutput.Write(SI_NEWLINE);
            a_oOutput.Write(SI_NEWLINE);
            bNeedNewLine = false;
        }

        // write the section (unless there is no section name)
        if (*iSection->item) {
            if (!convert.ConvertToStore(iSection->item)) { return SI_FAIL; }
            a_oOutput.Write("[");
            a_oOutput.Write(convert.Data());
            a_oOutput.Write("]");
            a_oOutput.Write(SI_NEWLINE);
        }

        // get all of the keys sorted in load order
        Entries oKeys;
        GetAllKeys(iSection->item, oKeys);
        oKeys.sort(typename Entry::LoadOrder());

        // write all keys and values
        typename Entries::const_iterator iKey = oKeys.begin();
        for (; iKey != oKeys.end(); ++iKey) {
            // get all values for this key
            Entries oValues;
            GetAllValues(iSection->item, iKey->item, oValues);

            typename Entries::const_iterator iValue = oValues.begin();
            for (; iValue != oValues.end(); ++iValue) {
                // write out the comment if there is one
                if (iValue->comment) {
                    a_oOutput.Write(SI_NEWLINE);
                    if (!OutputMultiLineText(a_oOutput, convert, iValue->comment)) { return SI_FAIL; }
                }

                // write the key
                if (!convert.ConvertToStore(iKey->item)) { return SI_FAIL; }
                a_oOutput.Write(convert.Data());

                // write the value
                if (!convert.ConvertToStore(iValue->item)) { return SI_FAIL; }
                a_oOutput.Write(m_bSpaces ? " = " : "=");
                if (m_bAllowMultiLine && IsMultiLineData(iValue->item)) {
                    // multi-line data needs to be processed specially to ensure
                    // that we use the correct newline format for the current system
                    a_oOutput.Write("<<<END_OF_TEXT" SI_NEWLINE);
                    if (!OutputMultiLineText(a_oOutput, convert, iValue->item)) { return SI_FAIL; }
                    a_oOutput.Write("END_OF_TEXT");
                } else {
                    a_oOutput.Write(convert.Data());
                }
                a_oOutput.Write(SI_NEWLINE);
            }
        }

        bNeedNewLine = true;
    }

    return SI_OK;
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::OutputMultiLineText(Writer &a_oOutput, Converter &a_oConverter,
                                                                const Character *a_pText) const
{
    const Character *pEndOfLine;
    Character cEndOfLineChar = *a_pText;
    while (cEndOfLineChar) {
        // find the end of this line
        pEndOfLine = a_pText;
        for (; *pEndOfLine && *pEndOfLine != '\n'; ++pEndOfLine) /*loop*/
            ;
        cEndOfLineChar = *pEndOfLine;

        // temporarily null terminate, convert and output the line
        *const_cast<Character *>(pEndOfLine) = 0;
        if (!a_oConverter.ConvertToStore(a_pText)) { return false; }
        *const_cast<Character *>(pEndOfLine) = cEndOfLineChar;
        a_pText += (pEndOfLine - a_pText) + 1;
        a_oOutput.Write(a_oConverter.Data());
        a_oOutput.Write(SI_NEWLINE);
    }
    return true;
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::Delete(const Character *a_pSection, const Character *a_pKey,
                                                   bool a_bRemoveEmpty)
{
    return DeleteValue(a_pSection, a_pKey, NULL, a_bRemoveEmpty);
}

template <class Character, class Compare, class Encoder>
bool IniTempl<Character, Compare, Encoder>::DeleteValue(const Character *a_pSection, const Character *a_pKey,
                                                        const Character *a_pValue, bool a_bRemoveEmpty)
{
    if (!a_pSection) { return false; }

    typename Sections::iterator iSection = m_data.find(a_pSection);
    if (iSection == m_data.end()) { return false; }

    // remove a single key if we have a keyname
    if (a_pKey) {
        typename Pairs::iterator iKeyVal = iSection->second.find(a_pKey);
        if (iKeyVal == iSection->second.end()) { return false; }

        const static Compare isLess = Compare();

        // remove any copied strings and then the key
        typename Pairs::iterator iDelete;
        bool bDeleted = false;
        do {
            iDelete = iKeyVal++;

            if (a_pValue == NULL
                || (isLess(a_pValue, iDelete->second) == false && isLess(iDelete->second, a_pValue) == false)) {
                DeleteString(iDelete->first.item);
                DeleteString(iDelete->second);
                iSection->second.erase(iDelete);
                bDeleted = true;
            }
        } while (iKeyVal != iSection->second.end() && !IsLess(a_pKey, iKeyVal->first.item));

        if (!bDeleted) { return false; }

        // done now if the section is not empty or we are not pruning away
        // the empty sections. Otherwise let it fall through into the section
        // deletion code
        if (!a_bRemoveEmpty || !iSection->second.empty()) { return true; }
    } else {
        // delete all copied strings from this section. The actual
        // entries will be removed when the section is removed.
        typename Pairs::iterator iKeyVal = iSection->second.begin();
        for (; iKeyVal != iSection->second.end(); ++iKeyVal) {
            DeleteString(iKeyVal->first.item);
            DeleteString(iKeyVal->second);
        }
    }

    // delete the section itself
    DeleteString(iSection->first.item);
    m_data.erase(iSection);

    return true;
}

template <class Character, class Compare, class Encoder>
void IniTempl<Character, Compare, Encoder>::DeleteString(const Character *a_pString)
{
    // strings may exist either inside the data block, or they will be
    // individually allocated and stored in m_strings. We only physically
    // delete those stored in m_strings.
    if (a_pString < m_pData || a_pString >= m_pData + m_uDataLen) {
        typename Entries::iterator i = m_strings.begin();
        for (; i != m_strings.end(); ++i) {
            if (a_pString == i->item) {
                delete[] const_cast<Character *>(i->item);
                m_strings.erase(i);
                break;
            }
        }
    }
}
} // namespace config


#if 0
// ---------------------------------------------------------------------------
//                              CONVERSION FUNCTIONS
// ---------------------------------------------------------------------------

// Defines the conversion classes for different libraries. Before including
// SimpleIni.h, set the converter that you wish you use by defining one of the
// following symbols.
//
//  SI_NO_CONVERSION        Do not make the "W" wide character version of the
//                          library available. Only CSimpleIniA etc is defined.
//  SI_CONVERT_GENERIC      Use the Unicode reference conversion library in
//                          the accompanying files ConvertUTF.h/c
//  SI_CONVERT_ICU          Use the IBM ICU conversion library. Requires
//                          ICU headers on include path and icuuc.lib
//  SI_CONVERT_WIN32        Use the Win32 API functions for conversion.

#if !defined(SI_NO_CONVERSION) && !defined(SI_CONVERT_GENERIC) && !defined(SI_CONVERT_WIN32) && !defined(SI_CONVERT_ICU)
#ifdef _WIN32
#define SI_CONVERT_WIN32
#else
#define SI_CONVERT_GENERIC
#endif
#endif

/**
 * Generic case-sensitive less than comparison. This class returns numerically
 * ordered ASCII case-sensitive text for all possible sizes and types of
 * Character.
 */
template <class Character>
struct SI_GenericCase {
    bool operator()(const Character *pLeft, const Character *pRight) const
    {
        long cmp;
        for (; *pLeft && *pRight; ++pLeft, ++pRight) {
            cmp = (long)*pLeft - (long)*pRight;
            if (cmp != 0) { return cmp < 0; }
        }
        return *pRight != 0;
    }
};

/**
 * Generic ASCII case-insensitive less than comparison. This class returns
 * numerically ordered ASCII case-insensitive text for all possible sizes
 * and types of Character. It is not safe for MBCS text comparison where
 * ASCII A-Z characters are used in the encoding of multi-byte characters.
 */
template <class Character>
struct SI_GenericNoCase {
    inline Character locase(Character ch) const
    {
        return (ch < 'A' || ch > 'Z') ? ch : (ch - 'A' + 'a');
    }
    bool operator()(const Character *pLeft, const Character *pRight) const
    {
        long cmp;
        for (; *pLeft && *pRight; ++pLeft, ++pRight) {
            cmp = (long)locase(*pLeft) - (long)locase(*pRight);
            if (cmp != 0) { return cmp < 0; }
        }
        return *pRight != 0;
    }
};

/**
 * Null conversion class for MBCS/UTF-8 to char (or equivalent).
 */
template <class Character>
class SI_ConvertA {
    bool m_bStoreIsUtf8;

  protected:
    SI_ConvertA() {}

  public:
    SI_ConvertA(bool use_utf8) : m_bStoreIsUtf8(use_utf8) {}

    /* copy and assignment */
    SI_ConvertA(const SI_ConvertA &rhs)
    {
        operator=(rhs);
    }
    SI_ConvertA &operator=(const SI_ConvertA &rhs)
    {
        m_bStoreIsUtf8 = rhs.m_bStoreIsUtf8;
        return *this;
    }

    /** Calculate the number of Character required for converting the input
     * from the storage format. The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to Character.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                      must be the actual length of the data, including
     *                      NULL byte if NULL terminated string is required.
     * @return              Number of Character required by the string when
     *                      converted. If there are embedded NULL bytes in the
     *                      input data, only the string up and not including
     *                      the NULL byte will be converted.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeFromStore(const char *a_pInputData, size_t a_uInputDataLen)
    {
        (void)a_pInputData;
        assert(a_uInputDataLen != (size_t)-1);

        // ASCII/MBCS/UTF-8 needs no conversion
        return a_uInputDataLen;
    }

    /** Convert the input string from the storage format to Character.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to Character.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                      must be the actual length of the data, including
     *                      NULL byte if NULL terminated string is required.
     * @param a_pOutputData Pointer to the output buffer to received the
     *                      converted data.
     * @param a_uOutputDataSize Size of the output buffer in Character.
     * @return              true if all of the input data was successfully
     *                      converted.
     */
    bool ConvertFromStore(const char *a_pInputData, size_t a_uInputDataLen, Character *a_pOutputData,
                          size_t a_uOutputDataSize)
    {
        // ASCII/MBCS/UTF-8 needs no conversion
        if (a_uInputDataLen > a_uOutputDataSize) { return false; }
        memcpy(a_pOutputData, a_pInputData, a_uInputDataLen);
        return true;
    }

    /** Calculate the number of char required by the storage format of this
     * data. The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  NULL terminated string to calculate the number of
     *                      bytes required to be converted to storage format.
     * @return              Number of bytes required by the string when
     *                      converted to storage format. This size always
     *                      includes space for the terminating NULL character.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeToStore(const Character *a_pInputData)
    {
        // ASCII/MBCS/UTF-8 needs no conversion
        return strlen((const char *)a_pInputData) + 1;
    }

    /** Convert the input string to the storage format of this data.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  NULL terminated source string to convert. All of
     *                      the data will be converted including the
     *                      terminating NULL character.
     * @param a_pOutputData Pointer to the buffer to receive the converted
     *                      string.
     * @param a_uOutputDataSize Size of the output buffer in char.
     * @return              true if all of the input data, including the
     *                      terminating NULL character was successfully
     *                      converted.
     */
    bool ConvertToStore(const Character *a_pInputData, char *a_pOutputData, size_t a_uOutputDataSize)
    {
        // calc input string length (Character type and size independent)
        size_t uInputLen = strlen((const char *)a_pInputData) + 1;
        if (uInputLen > a_uOutputDataSize) { return false; }

        // ascii/UTF-8 needs no conversion
        memcpy(a_pOutputData, a_pInputData, uInputLen);
        return true;
    }
};


// ---------------------------------------------------------------------------
//                              SI_CONVERT_GENERIC
// ---------------------------------------------------------------------------
#ifdef SI_CONVERT_GENERIC

#define SI_Case   SI_GenericCase
#define SI_NoCase SI_GenericNoCase

#include <wchar.h>
#include "ConvertUTF.h"

/**
 * Converts UTF-8 to a wchar_t (or equivalent) using the Unicode reference
 * library functions. This can be used on all platforms.
 */
template <class Character>
class SI_ConvertW {
    bool m_bStoreIsUtf8;

  protected:
    SI_ConvertW() {}

  public:
    SI_ConvertW(bool use_utf8) : m_bStoreIsUtf8(use_utf8) {}

    /* copy and assignment */
    SI_ConvertW(const SI_ConvertW &rhs)
    {
        operator=(rhs);
    }
    SI_ConvertW &operator=(const SI_ConvertW &rhs)
    {
        m_bStoreIsUtf8 = rhs.m_bStoreIsUtf8;
        return *this;
    }

    /** Calculate the number of Character required for converting the input
     * from the storage format. The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to Character.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                      must be the actual length of the data, including
     *                      NULL byte if NULL terminated string is required.
     * @return              Number of Character required by the string when
     *                      converted. If there are embedded NULL bytes in the
     *                      input data, only the string up and not including
     *                      the NULL byte will be converted.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeFromStore(const char *a_pInputData, size_t a_uInputDataLen)
    {
        assert(a_uInputDataLen != (size_t)-1);

        if (m_bStoreIsUtf8) {
            // worst case scenario for UTF-8 to wchar_t is 1 char -> 1 wchar_t
            // so we just return the same number of characters required as for
            // the source text.
            return a_uInputDataLen;
        }

#if defined(SI_NO_MBSTOWCS_NULL) || (!defined(_MSC_VER) && !defined(_linux))
        // fall back processing for platforms that don't support a NULL dest to mbstowcs
        // worst case scenario is 1:1, this will be a sufficient buffer size
        (void)a_pInputData;
        return a_uInputDataLen;
#else
        // get the actual required buffer size
        return mbstowcs(NULL, a_pInputData, a_uInputDataLen);
#endif
    }

    /** Convert the input string from the storage format to Character.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to Character.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                       must be the actual length of the data, including
     *                       NULL byte if NULL terminated string is required.
     * @param a_pOutputData Pointer to the output buffer to received the
     *                       converted data.
     * @param a_uOutputDataSize Size of the output buffer in Character.
     * @return              true if all of the input data was successfully
     *                       converted.
     */
    bool ConvertFromStore(const char *a_pInputData, size_t a_uInputDataLen, Character *a_pOutputData,
                          size_t a_uOutputDataSize)
    {
        if (m_bStoreIsUtf8) {
            // This uses the Unicode reference implementation to do the
            // conversion from UTF-8 to wchar_t. The required files are
            // ConvertUTF.h and ConvertUTF.c which should be included in
            // the distribution but are publically available from unicode.org
            // at http://www.unicode.org/Public/PROGRAMS/CVTUTF/
            ConversionResult retval;
            const UTF8 *pUtf8 = (const UTF8 *)a_pInputData;
            if (sizeof(wchar_t) == sizeof(UTF32)) {
                UTF32 *pUtf32 = (UTF32 *)a_pOutputData;
                retval = ConvertUTF8toUTF32(&pUtf8, pUtf8 + a_uInputDataLen, &pUtf32, pUtf32 + a_uOutputDataSize,
                                            lenientConversion);
            } else if (sizeof(wchar_t) == sizeof(UTF16)) {
                UTF16 *pUtf16 = (UTF16 *)a_pOutputData;
                retval = ConvertUTF8toUTF16(&pUtf8, pUtf8 + a_uInputDataLen, &pUtf16, pUtf16 + a_uOutputDataSize,
                                            lenientConversion);
            }
            return retval == conversionOK;
        }

        // convert to wchar_t
        size_t retval = mbstowcs(a_pOutputData, a_pInputData, a_uOutputDataSize);
        return retval != (size_t)(-1);
    }

    /** Calculate the number of char required by the storage format of this
     * data. The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  NULL terminated string to calculate the number of
     *                       bytes required to be converted to storage format.
     * @return              Number of bytes required by the string when
     *                       converted to storage format. This size always
     *                       includes space for the terminating NULL character.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeToStore(const Character *a_pInputData)
    {
        if (m_bStoreIsUtf8) {
            // worst case scenario for wchar_t to UTF-8 is 1 wchar_t -> 6 char
            size_t uLen = 0;
            while (a_pInputData[uLen]) { ++uLen; }
            return (6 * uLen) + 1;
        } else {
            size_t uLen = wcstombs(NULL, a_pInputData, 0);
            if (uLen == (size_t)(-1)) { return uLen; }
            return uLen + 1; // include NULL terminator
        }
    }

    /** Convert the input string to the storage format of this data.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  NULL terminated source string to convert. All of
     *                       the data will be converted including the
     *                       terminating NULL character.
     * @param a_pOutputData Pointer to the buffer to receive the converted
     *                       string.
     * @param a_uOutputDataSize Size of the output buffer in char.
     * @return              true if all of the input data, including the
     *                       terminating NULL character was successfully
     *                       converted.
     */
    bool ConvertToStore(const Character *a_pInputData, char *a_pOutputData, size_t a_uOutputDataSize)
    {
        if (m_bStoreIsUtf8) {
            // calc input string length (Character type and size independent)
            size_t uInputLen = 0;
            while (a_pInputData[uInputLen]) { ++uInputLen; }
            ++uInputLen; // include the NULL char

            // This uses the Unicode reference implementation to do the
            // conversion from wchar_t to UTF-8. The required files are
            // ConvertUTF.h and ConvertUTF.c which should be included in
            // the distribution but are publically available from unicode.org
            // at http://www.unicode.org/Public/PROGRAMS/CVTUTF/
            ConversionResult retval;
            UTF8 *pUtf8 = (UTF8 *)a_pOutputData;
            if (sizeof(wchar_t) == sizeof(UTF32)) {
                const UTF32 *pUtf32 = (const UTF32 *)a_pInputData;
                retval = ConvertUTF32toUTF8(&pUtf32, pUtf32 + uInputLen, &pUtf8, pUtf8 + a_uOutputDataSize,
                                            lenientConversion);
            } else if (sizeof(wchar_t) == sizeof(UTF16)) {
                const UTF16 *pUtf16 = (const UTF16 *)a_pInputData;
                retval = ConvertUTF16toUTF8(&pUtf16, pUtf16 + uInputLen, &pUtf8, pUtf8 + a_uOutputDataSize,
                                            lenientConversion);
            }
            return retval == conversionOK;
        } else {
            size_t retval = wcstombs(a_pOutputData, a_pInputData, a_uOutputDataSize);
            return retval != (size_t)-1;
        }
    }
};

#endif // SI_CONVERT_GENERIC


// ---------------------------------------------------------------------------
//                              SI_CONVERT_ICU
// ---------------------------------------------------------------------------
#ifdef SI_CONVERT_ICU

#define SI_Case   SI_GenericCase
#define SI_NoCase SI_GenericNoCase

#include <unicode/ucnv.h>

/**
 * Converts MBCS/UTF-8 to UChar using ICU. This can be used on all platforms.
 */
template <class Character>
class SI_ConvertW {
    const char *m_pEncoding;
    UConverter *m_pConverter;

  protected:
    SI_ConvertW() : m_pEncoding(NULL), m_pConverter(NULL) {}

  public:
    SI_ConvertW(bool use_utf8) : m_pConverter(NULL)
    {
        m_pEncoding = use_utf8 ? "UTF-8" : NULL;
    }

    /* copy and assignment */
    SI_ConvertW(const SI_ConvertW &rhs)
    {
        operator=(rhs);
    }
    SI_ConvertW &operator=(const SI_ConvertW &rhs)
    {
        m_pEncoding = rhs.m_pEncoding;
        m_pConverter = NULL;
        return *this;
    }
    ~SI_ConvertW()
    {
        if (m_pConverter) ucnv_close(m_pConverter);
    }

    /** Calculate the number of UChar required for converting the input
     * from the storage format. The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to UChar.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                      must be the actual length of the data, including
     *                      NULL byte if NULL terminated string is required.
     * @return              Number of UChar required by the string when
     *                      converted. If there are embedded NULL bytes in the
     *                      input data, only the string up and not including
     *                      the NULL byte will be converted.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeFromStore(const char *a_pInputData, size_t a_uInputDataLen)
    {
        assert(a_uInputDataLen != (size_t)-1);

        UErrorCode nError;

        if (!m_pConverter) {
            nError = U_ZERO_ERROR;
            m_pConverter = ucnv_open(m_pEncoding, &nError);
            if (U_FAILURE(nError)) { return (size_t)-1; }
        }

        nError = U_ZERO_ERROR;
        int32_t nLen = ucnv_toUChars(m_pConverter, NULL, 0, a_pInputData, (int32_t)a_uInputDataLen, &nError);
        if (U_FAILURE(nError) && nError != U_BUFFER_OVERFLOW_ERROR) { return (size_t)-1; }

        return (size_t)nLen;
    }

    /** Convert the input string from the storage format to UChar.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to UChar.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                      must be the actual length of the data, including
     *                      NULL byte if NULL terminated string is required.
     * @param a_pOutputData Pointer to the output buffer to received the
     *                      converted data.
     * @param a_uOutputDataSize Size of the output buffer in UChar.
     * @return              true if all of the input data was successfully
     *                      converted.
     */
    bool ConvertFromStore(const char *a_pInputData, size_t a_uInputDataLen, UChar *a_pOutputData,
                          size_t a_uOutputDataSize)
    {
        UErrorCode nError;

        if (!m_pConverter) {
            nError = U_ZERO_ERROR;
            m_pConverter = ucnv_open(m_pEncoding, &nError);
            if (U_FAILURE(nError)) { return false; }
        }

        nError = U_ZERO_ERROR;
        ucnv_toUChars(m_pConverter, a_pOutputData, (int32_t)a_uOutputDataSize, a_pInputData, (int32_t)a_uInputDataLen,
                      &nError);
        if (U_FAILURE(nError)) { return false; }

        return true;
    }

    /** Calculate the number of char required by the storage format of this
     * data. The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  NULL terminated string to calculate the number of
     *                      bytes required to be converted to storage format.
     * @return              Number of bytes required by the string when
     *                      converted to storage format. This size always
     *                      includes space for the terminating NULL character.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeToStore(const UChar *a_pInputData)
    {
        UErrorCode nError;

        if (!m_pConverter) {
            nError = U_ZERO_ERROR;
            m_pConverter = ucnv_open(m_pEncoding, &nError);
            if (U_FAILURE(nError)) { return (size_t)-1; }
        }

        nError = U_ZERO_ERROR;
        int32_t nLen = ucnv_fromUChars(m_pConverter, NULL, 0, a_pInputData, -1, &nError);
        if (U_FAILURE(nError) && nError != U_BUFFER_OVERFLOW_ERROR) { return (size_t)-1; }

        return (size_t)nLen + 1;
    }

    /** Convert the input string to the storage format of this data.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  NULL terminated source string to convert. All of
     *                      the data will be converted including the
     *                      terminating NULL character.
     * @param a_pOutputData Pointer to the buffer to receive the converted
     *                      string.
     * @param a_pOutputDataSize Size of the output buffer in char.
     * @return              true if all of the input data, including the
     *                      terminating NULL character was successfully
     *                      converted.
     */
    bool ConvertToStore(const UChar *a_pInputData, char *a_pOutputData, size_t a_uOutputDataSize)
    {
        UErrorCode nError;

        if (!m_pConverter) {
            nError = U_ZERO_ERROR;
            m_pConverter = ucnv_open(m_pEncoding, &nError);
            if (U_FAILURE(nError)) { return false; }
        }

        nError = U_ZERO_ERROR;
        ucnv_fromUChars(m_pConverter, a_pOutputData, (int32_t)a_uOutputDataSize, a_pInputData, -1, &nError);
        if (U_FAILURE(nError)) { return false; }

        return true;
    }
};

#endif // SI_CONVERT_ICU


// ---------------------------------------------------------------------------
//                              SI_CONVERT_WIN32
// ---------------------------------------------------------------------------
#ifdef SI_CONVERT_WIN32

#define SI_Case SI_GenericCase

// Windows CE doesn't have errno or MBCS libraries
#ifdef _WIN32_WCE
#ifndef SI_NO_MBCS
#define SI_NO_MBCS
#endif
#endif

#include <windows.h>
#ifdef SI_NO_MBCS
#define SI_NoCase SI_GenericNoCase
#else // !SI_NO_MBCS
/**
 * Case-insensitive comparison class using Win32 MBCS functions. This class
 * returns a case-insensitive semi-collation order for MBCS text. It may not
 * be safe for UTF-8 text returned in char format as we don't know what
 * characters will be folded by the function! Therefore, if you are using
 * Character == char and SetUnicode(true), then you need to use the generic
 * SI_NoCase class instead.
 */
#include <mbstring.h>
template <class Character>
struct SI_NoCase {
    bool operator()(const Character *pLeft, const Character *pRight) const
    {
        if (sizeof(Character) == sizeof(char)) {
            return _mbsicmp((const unsigned char *)pLeft, (const unsigned char *)pRight) < 0;
        }
        if (sizeof(Character) == sizeof(wchar_t)) {
            return _wcsicmp((const wchar_t *)pLeft, (const wchar_t *)pRight) < 0;
        }
        return SI_GenericNoCase<Character>()(pLeft, pRight);
    }
};
#endif // SI_NO_MBCS

/**
 * Converts MBCS and UTF-8 to a wchar_t (or equivalent) on Windows. This uses
 * only the Win32 functions and doesn't require the external Unicode UTF-8
 * conversion library. It will not work on Windows 95 without using Microsoft
 * Layer for Unicode in your application.
 */
template <class Character>
class SI_ConvertW {
    UINT m_uCodePage;

  protected:
    SI_ConvertW() {}

  public:
    SI_ConvertW(bool use_utf8)
    {
        m_uCodePage = use_utf8 ? CP_UTF8 : CP_ACP;
    }

    /* copy and assignment */
    SI_ConvertW(const SI_ConvertW &rhs)
    {
        operator=(rhs);
    }
    SI_ConvertW &operator=(const SI_ConvertW &rhs)
    {
        m_uCodePage = rhs.m_uCodePage;
        return *this;
    }

    /** Calculate the number of Character required for converting the input
     * from the storage format. The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to Character.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                      must be the actual length of the data, including
     *                      NULL byte if NULL terminated string is required.
     * @return              Number of Character required by the string when
     *                      converted. If there are embedded NULL bytes in the
     *                      input data, only the string up and not including
     *                      the NULL byte will be converted.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeFromStore(const char *a_pInputData, size_t a_uInputDataLen)
    {
        assert(a_uInputDataLen != (size_t)-1);

        int retval = MultiByteToWideChar(m_uCodePage, 0, a_pInputData, (int)a_uInputDataLen, 0, 0);
        return (size_t)(retval > 0 ? retval : -1);
    }

    /** Convert the input string from the storage format to Character.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  Data in storage format to be converted to Character.
     * @param a_uInputDataLen Length of storage format data in bytes. This
     *                      must be the actual length of the data, including
     *                      NULL byte if NULL terminated string is required.
     * @param a_pOutputData Pointer to the output buffer to received the
     *                      converted data.
     * @param a_uOutputDataSize Size of the output buffer in Character.
     * @return              true if all of the input data was successfully
     *                      converted.
     */
    bool ConvertFromStore(const char *a_pInputData, size_t a_uInputDataLen, Character *a_pOutputData,
                          size_t a_uOutputDataSize)
    {
        int nSize = MultiByteToWideChar(m_uCodePage, 0, a_pInputData, (int)a_uInputDataLen, (wchar_t *)a_pOutputData,
                                        (int)a_uOutputDataSize);
        return (nSize > 0);
    }

    /** Calculate the number of char required by the storage format of this
     * data. The storage format is always UTF-8.
     *
     * @param a_pInputData  NULL terminated string to calculate the number of
     *                      bytes required to be converted to storage format.
     * @return              Number of bytes required by the string when
     *                      converted to storage format. This size always
     *                      includes space for the terminating NULL character.
     * @return              -1 cast to size_t on a conversion error.
     */
    size_t SizeToStore(const Character *a_pInputData)
    {
        int retval = WideCharToMultiByte(m_uCodePage, 0, (const wchar_t *)a_pInputData, -1, 0, 0, 0, 0);
        return (size_t)(retval > 0 ? retval : -1);
    }

    /** Convert the input string to the storage format of this data.
     * The storage format is always UTF-8 or MBCS.
     *
     * @param a_pInputData  NULL terminated source string to convert. All of
     *                      the data will be converted including the
     *                      terminating NULL character.
     * @param a_pOutputData Pointer to the buffer to receive the converted
     *                      string.
     * @param a_pOutputDataSize Size of the output buffer in char.
     * @return              true if all of the input data, including the
     *                      terminating NULL character was successfully
     *                      converted.
     */
    bool ConvertToStore(const Character *a_pInputData, char *a_pOutputData, size_t a_uOutputDataSize)
    {
        int retval = WideCharToMultiByte(m_uCodePage, 0, (const wchar_t *)a_pInputData, -1, a_pOutputData,
                                         (int)a_uOutputDataSize, 0, 0);
        return retval > 0;
    }
};

#endif // SI_CONVERT_WIN32



// ---------------------------------------------------------------------------
//                              SI_NO_CONVERSION
// ---------------------------------------------------------------------------
#ifdef SI_NO_CONVERSION

#define SI_Case   SI_GenericCase
#define SI_NoCase SI_GenericNoCase

#endif // SI_NO_CONVERSION



// ---------------------------------------------------------------------------
//                                  TYPE DEFINITIONS
// ---------------------------------------------------------------------------

typedef IniTempl<char, SI_NoCase<char>, SI_ConvertA<char> > CSimpleIniA;
typedef IniTempl<char, SI_Case<char>, SI_ConvertA<char> > CSimpleIniCaseA;

#if defined(SI_NO_CONVERSION)
// if there is no wide char conversion then we don't need to define the
// widechar "W" versions of CSimpleIni
#define CSimpleIni     CSimpleIniA
#define CSimpleIniCase CSimpleIniCaseA
#define SI_NEWLINE     SI_NEWLINE
#else
#if defined(SI_CONVERT_ICU)
typedef IniTempl<UChar, SI_NoCase<UChar>, SI_ConvertW<UChar> > CSimpleIniW;
typedef IniTempl<UChar, SI_Case<UChar>, SI_ConvertW<UChar> > CSimpleIniCaseW;
#else
typedef IniTempl<wchar_t, SI_NoCase<wchar_t>, SI_ConvertW<wchar_t> > CSimpleIniW;
typedef IniTempl<wchar_t, SI_Case<wchar_t>, SI_ConvertW<wchar_t> > CSimpleIniCaseW;
#endif

#ifdef _UNICODE
#define CSimpleIni     CSimpleIniW
#define CSimpleIniCase CSimpleIniCaseW
#define SI_NEWLINE     SI_NEWLINE_W
#else // !_UNICODE
#define CSimpleIni     CSimpleIniA
#define CSimpleIniCase CSimpleIniCaseA
#define SI_NEWLINE     SI_NEWLINE
#endif // _UNICODE
#endif
#endif
