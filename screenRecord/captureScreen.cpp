/**
* 截屏测试
*/
#include "extern_function.h"


#include <iostream>
#include <fstream>

ExternFunction externFunction;

int main(int argc, char *argv[]) {
    // buff 存图片数据
    while (true){
        vector<uint8_t> buf;
        // 截屏
        int r = externFunction.captureScreen(1, 100, &buf);
        if (!r) {
            printf("截屏失败\n");
            return 1;
        }
      /*  std::string filename = "output.jpg";
        std::ofstream outputFile(filename, std::ios::binary);
        if (!outputFile) {
            printf("打开文件失败\n");
            return 1;
        }*/
        printf("size:%zu\n", buf.size());
        buf.clear();

    }
   /* vector<uint8_t> buf;
    // 截屏
    int r = externFunction.captureScreen(1, 100, &buf);
    if (!r) {
        printf("截屏失败\n");
        return 1;
    }
    std::string filename = "output.jpg";
    std::ofstream outputFile(filename, std::ios::binary);
    if (!outputFile) {
        printf("打开文件失败\n");
        return 1;
    }
    outputFile.write(reinterpret_cast<const char *>(buf.data()), buf.size());
    outputFile.close();
    if (outputFile.fail()) {
        printf("写入文件失败\n");
        return 1;
    }
    printf("size:%zu\n", buf.size());*/
}