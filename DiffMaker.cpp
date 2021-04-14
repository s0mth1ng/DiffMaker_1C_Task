#include "DiffMaker.h"

#include <sstream>
#include <cstring>
#include <fstream>

namespace DiffMaker {
    std::ostream &operator<<(std::ostream &out, const BlockDifference &diff) {
        char *idInHex = ToHex(diff.blockId);
        out.write(idInHex, sizeof(size_t));
        char *typeInHex = new char[1];
        typeInHex[0] = static_cast<int>(diff.type);
        out.write(typeInHex, 1);
        if (diff.type != BlockDifference::DifferenceType::Deleted) {
            out.write(diff.updatedData, diff.size);
        }
        delete[] idInHex;
        delete[] typeInHex;
        return out;
    }

    char *ToHex(size_t index) {
        size_t size = sizeof(size_t);
        char *data = new char[size];
        for (size_t i = 0; i < size; ++i) {
            data[size - i - 1] = index % 16;
            index >>= 4;
        }
        return data;
    }

    size_t GetFileSize(std::istream &file) {
        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);
        return size;
    }

    void
    CreateDiff(const std::string &oldFileName, const std::string &newFileName, std::string diffFileName) {
        if (diffFileName.empty()) {
            diffFileName = oldFileName + "_" + newFileName + "_diff";
        }

        std::ifstream oldFile(oldFileName, std::istream::binary);
        size_t oldFileSize = GetFileSize(oldFile);

        std::ifstream newFile(newFileName, std::istream::binary);
        size_t newFileSize = GetFileSize(newFile);

        std::ofstream diffFile(diffFileName, std::ostream::binary);

        size_t blockStart = 0;
        while (blockStart + BLOCK_SIZE <= oldFileSize && blockStart + BLOCK_SIZE <= newFileSize) {
            char *oldBlockData = new char[BLOCK_SIZE];
            oldFile.read(oldBlockData, BLOCK_SIZE);
            char *newBlockData = new char[BLOCK_SIZE];
            newFile.read(newBlockData, BLOCK_SIZE);
            if (strcmp(oldBlockData, newBlockData) != 0) {
                BlockDifference diff;
                diff.blockId = blockStart / BLOCK_SIZE;
                diff.type = BlockDifference::DifferenceType::Changed;
                diff.updatedData = newBlockData;
                diff.size = BLOCK_SIZE;
                diffFile << diff;
            }
            delete[] oldBlockData;
            delete[] newBlockData;
            blockStart += BLOCK_SIZE;
        }

        while (blockStart < newFileSize) {
            BlockDifference diff;
            diff.blockId = blockStart / BLOCK_SIZE;
            diff.type = BlockDifference::DifferenceType::Added;
            size_t size = std::min(newFileSize - blockStart, blockStart);
            diff.updatedData = new char[size];
            newFile.read(diff.updatedData, size);
            diff.size = size;
            diffFile << diff;
            blockStart += BLOCK_SIZE;
        }

        while (blockStart < oldFileSize) {
            BlockDifference diff;
            diff.blockId = blockStart / BLOCK_SIZE;
            diff.type = BlockDifference::DifferenceType::Deleted;
            diffFile << diff;
            blockStart += BLOCK_SIZE;
        }
    }
}  // namespace DiffMaker
