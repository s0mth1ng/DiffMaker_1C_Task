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

    std::istream &operator>>(std::istream &in, BlockDifference &diff) {
        char *idInHex = new char[sizeof(size_t)];
        in.read(idInHex, sizeof(size_t));
        diff.blockId = FromHex(idInHex);
        delete[] idInHex;

        char *typeInHex = new char[1];
        in.read(typeInHex, 1);
        diff.type = static_cast<BlockDifference::DifferenceType>(typeInHex[0]);
        if (diff.type != BlockDifference::DifferenceType::Deleted) {
            auto currentPos = in.tellg();
            in.seekg(0, std::istream::end);
            size_t size = std::min(static_cast<size_t>(in.tellg() - currentPos), BLOCK_SIZE);
            in.seekg(currentPos);
            diff.updatedData = new char[size];
            in.read(diff.updatedData, size);
            diff.size = size;
        }
        return in;
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
        while (blockStart < oldFileSize && blockStart < newFileSize) {
            size_t oldDataLeft = std::min(BLOCK_SIZE, oldFileSize - blockStart);
            size_t newDataLeft = std::min(BLOCK_SIZE, newFileSize - blockStart);
            char *oldBlockData = new char[oldDataLeft];
            oldFile.read(oldBlockData, oldDataLeft);
            char *newBlockData = new char[newDataLeft];
            newFile.read(newBlockData, newDataLeft);
            if (oldDataLeft != newDataLeft || strncmp(oldBlockData, newBlockData, oldDataLeft) != 0) {
                BlockDifference diff;
                diff.blockId = blockStart / BLOCK_SIZE;
                diff.type = BlockDifference::DifferenceType::Changed;
                diff.updatedData = newBlockData;
                diff.size = newDataLeft;
                diffFile << diff;
            }
            delete[] oldBlockData;
            delete[] newBlockData;
            blockStart += BLOCK_SIZE;
        }

        if (newFileSize > oldFileSize) {
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
        } else {
            while (blockStart < oldFileSize) {
                BlockDifference diff;
                diff.blockId = blockStart / BLOCK_SIZE;
                diff.type = BlockDifference::DifferenceType::Deleted;
                diffFile << diff;
                blockStart += BLOCK_SIZE;
            }
        }
    }

    void Recover(const std::string &oldFileName, const std::string &diffFileName, std::string newFileName) {
        if (newFileName.empty()) {
            newFileName = oldFileName + "_recovered";
        }
        std::ifstream oldFile(oldFileName, std::istream::binary);
        size_t oldFileSize = GetFileSize(oldFile);

        std::ifstream diffFile(diffFileName, std::ostream::binary);
        size_t diffFileSize = GetFileSize(oldFile);

        std::ofstream newFile(newFileName, std::istream::binary);

        BlockDifference diff;
        size_t blockStart = 0;
        while (diffFile >> diff) {
            std::cerr << (diff.type == BlockDifference::DifferenceType::Deleted) << ' ' << diff.blockId << ' '
                      << diff.size << std::endl;
            while (blockStart / BLOCK_SIZE != diff.blockId) {
                size_t size = std::min(oldFileSize - blockStart, BLOCK_SIZE);
                char *data = new char[size];
                oldFile.read(data, size);
                newFile.write(data, size);
                delete[] data;
                blockStart += BLOCK_SIZE;
            }
            if (diff.type != BlockDifference::DifferenceType::Deleted) {
                newFile.write(diff.updatedData, diff.size);
                delete[] diff.updatedData;
            }
            blockStart += BLOCK_SIZE;
        }
        while (blockStart < oldFileSize) {
            size_t size = std::min(oldFileSize - blockStart, BLOCK_SIZE);
            char *data = new char[size];
            oldFile.read(data, size);
            newFile.write(data, size);
            delete[] data;
            blockStart += BLOCK_SIZE;
        }
    }

    size_t FromHex(const char *hex) {
        size_t result = 0;
        size_t size = sizeof(size_t);
        for (size_t i = 0; i < size; ++i) {
            result <<= 4;
            result += hex[i];
        }
        return result;
    }
}  // namespace DiffMaker
