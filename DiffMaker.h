#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <optional>

namespace DiffMaker {

    const static size_t DEFAULT_BLOCK_SIZE = 32LU;
    static size_t BLOCK_SIZE = DEFAULT_BLOCK_SIZE;

    char *ToHex(size_t index);

    struct BlockDifference {
        enum DifferenceType {
            Deleted = 0,
            Changed = 1,
            Added = 2,
        };

        size_t blockId;
        DifferenceType type;
        char *updatedData = nullptr;
        size_t size;

        friend std::ostream &operator<<(std::ostream &out, const BlockDifference &diff);
    };

    void CreateDiff(const std::string &oldFileName, const std::string &newFileName,
                    std::string diffFileName = "");

    size_t GetFileSize(std::istream &file);

}  // namespace DiffMaker