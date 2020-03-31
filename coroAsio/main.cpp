#include "examples.h"
#include "Protocol.h"
#include "Result.h"

int main(int argc, char** argv)
{
    example1::main(argc, argv);
    tupleMain();
    //protoMain();
    return 0;

    example2::main(argc, argv);
    example3::main(argc, argv);
    example4::main(argc, argv);


}
