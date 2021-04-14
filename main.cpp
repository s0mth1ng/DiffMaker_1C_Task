#include <fstream>

#include "DiffMaker.h"

int main() {
    DiffMaker::CreateDiff("../binary_files/old.jpg", "../binary_files/new.jpg",
                          "../binary_files/diff");
    DiffMaker::Recover("../binary_files/old.jpg", "../binary_files/diff", "../binary_files/recovered.jpg");
    return 0;
}
