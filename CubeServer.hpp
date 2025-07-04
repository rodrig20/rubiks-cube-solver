#ifndef CUBE_SERVER_HPP
#define CUBE_SERVER_HPP

#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include "Robot.hpp"

// Forward declaration
class Robot;

class CubeServer {
   private:
    Robot* robot = nullptr;
    WebServer server;
    const char* ssid = "CubeSolver";
    const char* password = "#cubeSolver";

    void handleCapture();
    void serveFile(const String& path, const String& contentType);
    void setupRoutes();
    String getCubeStateString(
        const std::array<std::array<int, 3>, 26>& cubeState);

   public:
   CubeServer(Robot* robot_ref, int port);
   void handleClient();
   ~CubeServer();
};

#endif
