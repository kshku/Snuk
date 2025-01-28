#include "sstring.h"

#include "core/memory/memory.h"

/**
 * @brief Get the length of the string (similar to strlen).
 *
 * @param str String
 *
 * @return Returns the size of the stirng.
 */
u64 sStringLength(const char *restrict str) {
    u64 len = 0;
    while (str[len]) ++len;
    return len;
}

/**
 * @brief Concatenate two string and return malloced string.
 *
 * If the l1 or l2 is 0, calls sStringLength, else string of given length will
 * be used.
 *
 * @param str1 First string
 * @param str2 Second string
 * @param l1 Length of the first string or 0
 * @param l2 Length of the second string or 0
 * @param length The length of the concatenated string
 *
 * @return Returns the malloced string, user needs to call sFree once done.
 */
char *sStringConcat(const char *str1, const char *str2, u64 l1, u64 l2,
                    u64 *restrict length) {
    if (!l1) l1 = sStringLength(str1);
    if (!l2) l2 = sStringLength(str2);

    char *str = (char *)sCalloc(l1 + l2 + 1, sizeof(char));
    sMemCopy((void *)str, (void *)str1, l1 * sizeof(char));
    sMemCopy((void *)(str + l1), (void *)str2, l2 * sizeof(char));

    if (length) *length = l1 + l2;

    return str;
}

/**
 * @brief Check whether two strings are equal.
 *
 * If length is zero, it compares all the characters. If length is not zero,
 * then compares both strings upto given length.
 *
 * @param str1 String to compare
 * @param str2 String to compare
 * @param len Length of the strings to be compared
 *
 * @return Returns true if the strings are equal.
 */
b8 sStringEqual(const char *str1, const char *str2, u64 len) {
    if (str1 == str2) return true;
    if (len == 0) len = (u64)-1;

    u64 i = 0;

    for (i = 0; i < len && str1[i] && str2[i]; ++i)
        if (str1[i] != str2[i]) return false;

    if ((str1[i] || str2[i]) && i != len) return false;

    return true;
}

/**
 * @brief Copy the string.
 *
 * If len is 0, calls sStringLength, else will copy string upto length len.
 *
 * @param str The String to be copied
 * @param len length of the string or 0
 *
 * @return Returns the malloced copied string, user should call sFree.
 */
char *sStringCopy(const char *restrict str, u64 len) {
    if (!len) len = sStringLength(str);

    char *s = (char *)sCalloc((len + 1), sizeof(char));
    sMemCopy((void *)s, (void *)str, len * sizeof(char));

    return s;
}

// /**
//  * @brief Get the length of the string (similar to strlen).
//  *
//  * @param str String
//  *
//  * @return Returns the size of the stirng.
//  */
// u64 sStringLengthC16(const c16 *str) {
//     u64 len = 0;
//     while (str[len]) ++len;
//     return len;
// }

// /**
//  * @brief Concatenate two string and return malloced string.
//  *
//  * If the l1 or l2 is 0, calls sStringLength, else string of given length
//  will
//  * be used.
//  *
//  * @param str1 First string
//  * @param str2 Second string
//  * @param l1 Length of the first string or 0
//  * @param l2 Length of the second string or 0
//  * @param length The length of the concatenated string
//  *
//  * @return Returns the malloced string, user needs to call sFree once done.
//  */
// c16 *sStringConcatC16(const c16 *str1, const c16 *str2, u64 l1, u64 l2,
//                       u64 *length) {
//     if (!l1) l1 = sStringLengthC16(str1);
//     if (!l2) l2 = sStringLengthC16(str2);

//     c16 *str = (c16 *)sCalloc(l1 + l2 + 1, sizeof(c16));
//     sMemCopy((void *)str, (void *)str1, l1 * sizeof(c16));
//     sMemCopy((void *)(str + l1), (void *)str2, l2 * sizeof(c16));

//     if (length) *length = l1 + l2;

//     return str;
// }

// /**
//  * @brief Check whether two strings are equal.
//  *
//  * If length is zero, it compares all the characters. If length is not zero,
//  * then compares both strings upto given length.
//  *
//  * @param str1 String to compare
//  * @param str2 String to compare
//  * @param len Length of the strings to be compared
//  *
//  * @return Returns true if the strings are equal.
//  */
// b8 sStringEqualC16(const c16 *str1, const c16 *str2, u64 len) {
//     if (str1 == str2) return true;
//     if (len == 0) len = (u64)-1;

//     u64 i = 0;

//     for (i = 0; i < len && str1[i] && str2[i]; ++i)
//         if (str1[i] != str2[i]) return false;

//     if ((str1[i] || str2[i]) && i != len) return false;

//     return true;
// }

// /**
//  * @brief Copy the string.
//  *
//  * If len is 0, calls sStringLength, else will copy string upto length len.
//  *
//  * @param str The String to be copied
//  * @param len length of the string or 0
//  *
//  * @return Returns the malloced copied string, user should call sFree.
//  */
// c16 *sStringCopyC16(const c16 *str, u64 len) {
//     if (!len) len = sStringLengthC16(str);

//     c16 *s = (c16 *)sCalloc((len + 1), sizeof(c16));
//     sMemCopy((void *)s, (void *)str, len * sizeof(c16));

//     return s;
// }
