//
// Created by fgsqme on 2022/9/29.
//

#include "DataDec.h"
#include "TCPServer.h"
#include "ByteUtils.h"
#include "H264Decoder.h"
#include "ImageTexture.h"
#include "draw.h"
// 原始触摸方案
//#include "touch.h"
// 触摸穿透解决方案
#include "touch.h"

extern Draw *draw;

// 接收h264编码流，使用ffmpeg解码到imgui显示
int main(int argc, char *argv[]) {
    draw = new DrawVulkan(); // DrawVulkan /  DrawOpenGL

    if (!draw->initDraw(true)) {
        return -1;
    }
    initTouch();
    // h264解码工具
    H264Decoder decoder;
    // Tcp 服务
    TCPServer tcpServer(6656);

    // 4M缓存接收数据包用
    int bufferLen = 1024 * 1024 * 4;
    auto *buffer = new mbyte[bufferLen];

    while (true) {
        // 监听客户端
        TCPClient *tcpClient = tcpServer.accept();
        DataDec dataDec(buffer, bufferLen);
        ssize_t err = 0;
        while (true) {
            draw->drawBegin();
            // 接收头信息
            err = tcpClient->recvo(buffer, DataDec::headerSize());
            if (err != DataDec::headerSize()) {
                printf("headerSize error: %zd\n", err);
                break;
            }
            // 接收数据包
            int frameLenth = dataDec.getLength();
            // printf("frameLenth len: %d\n", frameLenth);
            err = tcpClient->recvo(buffer, DataDec::headerSize(), frameLenth, 0);
            if (err != frameLenth) {
                printf("Failed to get frame. length: %zd\n", err);
                break;
            }
            decoder.decode((unsigned char *) (buffer + DataDec::headerSize()), frameLenth);
            ImageTexture imageTexture(decoder.getDecBuffer(), decoder.getWidth(), decoder.getHeight());
            ImGui::Begin("record");
            ImVec2 imVec2 = ImVec2((float) decoder.getWidth(), (float) decoder.getHeight());
            ImGui::SetWindowSize(ImVec2(imVec2.x + 100, imVec2.y + 100));
            ImGui::Image((ImTextureID) imageTexture.getOpenglTexture(), imVec2);
            ImGui::End();
            draw->drawEnd();
        }
        tcpClient->close();
    }

    tcpServer.close();
    delete[] buffer;
    closeTouch();
    draw->shutdown();
    return 0;
}