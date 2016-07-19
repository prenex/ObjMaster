//
// Various common global helper functions
// Created by rthier on 2016.07.15..
//

#ifndef NFTSIMPLEPROJ_HELPERFUN_H
#define NFTSIMPLEPROJ_HELPERFUN_H

#include <string>

/**
 * Returns if the given keyword is a real existing prefix of the given target string.
 */
static bool isStartsWith(const std::string &targetString, const std::string keyword) {

    // If the keyword is longer than the read field descriptor line
    // then we cannot parse it!
    if(keyword.size() > targetString.size()) {
        return false;
    }

    // Otherwise check for the first mismatching character of the keyword string
    auto mismatch = std::mismatch(keyword.begin(), keyword.end(),
                                  targetString.begin(), targetString.end());

    // It is a prefix if the first mismatch is after the end of the keyword string!
    if(mismatch.first == keyword.end()){
        return true;
    } else {
        return false;
    }
}

#endif //NFTSIMPLEPROJ_HELPERFUN_H
