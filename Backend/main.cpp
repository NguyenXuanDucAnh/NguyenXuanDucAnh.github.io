#include <iostream>

// http library
#include "Lib/httplib.h"

// json library
#include <nlohmann/json.hpp>
using json = nlohmann::json;

//
#include <fstream>

// Thêm thư viện base64 decode — tự implement nhỏ gọn

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_decode(const std::string& encoded) {
    std::string decoded;
    int val = 0, bits = -8;
    for (unsigned char c : encoded) {
        if (c == '=') break;
        auto pos = base64_chars.find(c);
        if (pos == std::string::npos) continue;
        val = (val << 6) + pos;
        bits += 6;
        if (bits >= 0) {
            decoded += (char)((val >> bits) & 0xFF);
            bits -= 8;
        }
    }
    return decoded;
}

// code web server
void server ()
{
    httplib::Server svr;
    // svr.set_payload_max_length(100 * 1024 * 1024); // 100MB
    // svr.set_read_timeout(60, 0);
    // svr.set_keep_alive_max_count(0); // ← TẮT keep-alive, thử xem body có được đọc không

    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        std::ifstream file("../index.html");

        if (!file) {
            res.status = 404;
            res.set_content("File not found", "text/plain");
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        
        res.set_content(buffer.str(), "text/html");
    });

    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "application/json");
    });

    svr.Post("/api/upload", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        try {
            auto body = json::parse(req.body);

            std::string filename  = body["filename"];
            std::string mime_type = body["mime_type"];
            std::string b64data   = body["data"];

            // Bảo mật: bỏ path traversal
            auto slash = filename.find_last_of("/\\");
            if (slash != std::string::npos) filename = filename.substr(slash + 1);

            // Decode base64 → binary
            std::string file_content = base64_decode(b64data);

            // Lưu file
            std::ofstream ofs("../uploads/" + filename, std::ios::binary);
            if (!ofs) {
                res.status = 500;
                res.set_content(R"({"ok":false,"message":"Không thể lưu file"})", "application/json");
                return;
            }
            ofs.write(file_content.data(), file_content.size());
            ofs.close();

            json response = {
                {"ok",       true},
                {"message",  "Upload thành công"},
                {"filename", filename},
                {"size",     file_content.size()}
            };
            res.set_content(response.dump(), "application/json");

        } catch (const std::exception& e) {
            res.status = 400;
            json err = {{"ok", false}, {"message", e.what()}};
            res.set_content(err.dump(), "application/json");
        }
    });

    svr.listen("0.0.0.0", 8080);
}

int main (int argv, char * argc[]){

    server ();
    return 1;
}