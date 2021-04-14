#include <fstream>

#include "DiffMaker.h"

int main() {
    DiffMaker::CreateDiff("../binary_files/old.jpg", "../binary_files/new.jpg",
                          "../binary_files/diff");
    return 0;
}
