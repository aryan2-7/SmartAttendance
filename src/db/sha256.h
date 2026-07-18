#pragma once
#include <string>
#include <sstream>
#include <iomanip>

class SHA256 {
private:
    unsigned int state[8];
    unsigned char buffer[64];
    unsigned long long bitCount;

    unsigned int rotateRight(unsigned int value, unsigned int shift) {
        return (value >> shift) | (value << (32 - shift));
    }

    void transform() {
        unsigned int w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<unsigned int>(buffer[i * 4]) << 24) | 
                (static_cast<unsigned int>(buffer[i * 4 + 1]) << 16) | 
                 (static_cast<unsigned int>(buffer[i * 4 + 2]) << 8) | 
                static_cast<unsigned int>(buffer[i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            unsigned int s0 = rotateRight(w[i - 15], 7) ^ rotateRight(w[i - 15], 18) ^ (w[i - 15] >> 3);
            unsigned int s1 = rotateRight(w[i - 2], 17) ^ rotateRight(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        unsigned int a = state[0], b = state[1], c = state[2], d = state[3];
        unsigned int e = state[4], f = state[5], g = state[6], h = state[7];

        const unsigned int k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        for (int i = 0; i < 64; ++i) {
            unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
            unsigned int ch = (e & f) ^ (~e & g);
            unsigned int s0 = rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22);
            unsigned int s1 = rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25);
            unsigned int t1 = h + s1 + ch + k[i] + w[i];
            unsigned int t2 = s0 + maj;

            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }

public:
    SHA256() {
        state[0] = 0x6a09e667; state[1] = 0xbb67ae85; state[2] = 0x3c6ef372; state[3] = 0xa54ff53a;
        state[4] = 0x510e527f; state[5] = 0x9b05688c; state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
        bitCount = 0;
    }

    void update(const std::string& data) {
        for (char c : data) {
            buffer[(bitCount / 8) % 64] = static_cast<unsigned char>(c);
            bitCount += 8;
            if (bitCount % 512 == 0) transform();
        }
    }

    std::string finalize() {
        unsigned long long totalBits = bitCount;
        update("\x80");
        const std::string zeroByte(1, '\0');
        while ((bitCount / 8) % 64 != 56) {
            update(zeroByte);
        }
        for (int i = 7; i >= 0; --i) {
            buffer[56 + i] = static_cast<unsigned char>(totalBits >> (8 * (7 - i)));
        }
        transform();

        std::stringstream ss;
        for (int i = 0; i < 8; ++i) {
            ss << std::hex << std::setw(8) << std::setfill('0') << state[i];
        }
        return ss.str();
    }

    // Static helper for quick hashing
    static std::string hash(const std::string& input) {
        SHA256 sha;
        sha.update(input);
        return sha.finalize();
    }
};