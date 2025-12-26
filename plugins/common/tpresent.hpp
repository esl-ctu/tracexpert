// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Petr Socha (initial author)

#ifndef TPRESENT_H
#define TPRESENT_H

#include <cstdint>
#include <cstring>

class TPRESENT {
private:
    static const uint8_t sbox[16];
    static const uint8_t inv_sbox[16];

    static inline uint64_t load64_be(const uint8_t* p) {
        uint64_t v = 0; for (int i = 0; i < 8; ++i) v = (v << 8) | p[i]; return v;
    }
    static inline void store64_be(uint8_t* p, uint64_t v) {
        for (int i = 7; i >= 0; --i) { p[i] = (uint8_t)(v & 0xFF); v >>= 8; }
    }

    static inline uint64_t SBoxLayer(uint64_t s) {
        uint64_t out = 0;
        for (int i = 0; i < 16; ++i) {
            uint8_t v = (s >> ((15 - i) * 4)) & 0xF;
            out = (out << 4) | sbox[v];
        }
        return out;
    }

    static inline uint64_t InvSBoxLayer(uint64_t s) {
        uint64_t out = 0;
        for (int i = 0; i < 16; ++i) {
            uint8_t v = (s >> ((15 - i) * 4)) & 0xF;
            out = (out << 4) | inv_sbox[v];
        }
        return out;
    }

    static inline uint64_t PLayer(uint64_t s) {
        uint64_t out = 0;
        for (int i = 0; i < 63; ++i) {
            uint64_t bit = (s >> (63 - i)) & 1ULL;
            out |= bit << (63 - ((16 * i) % 63));
        }
        out |= (s & 1ULL);
        return out;
    }
    static inline uint64_t InvPLayer(uint64_t s) {
        uint64_t out = 0;
        for (int i = 0; i < 63; ++i) {
            uint64_t bit = (s >> (63 - ((16 * i) % 63))) & 1ULL;
            out |= bit << (63 - i);
        }
        out |= (s & 1ULL);
        return out;
    }

    static inline void rotl_bits(uint8_t* buf, int nbytes, int r) {
        r %= nbytes * 8; if (!r) return;
        int by = r / 8, bm = r % 8; uint8_t tmp[16];
        for (int i = 0; i < nbytes; ++i) tmp[i] = buf[(i + by) % nbytes];
        if (!bm) { memcpy(buf, tmp, nbytes); return; }
        for (int i = 0; i < nbytes; ++i)
            buf[i] = (uint8_t)((tmp[i] << bm) | (tmp[(i + 1) % nbytes] >> (8 - bm)));
    }

    static inline void KeySchedule80(const uint8_t* key, uint8_t* rk) {
        uint8_t K[10]; memcpy(K, key, 10);
        for (int r = 0; r < 32; ++r) {
            memcpy(rk + 8 * r, K, 8);
            if (r == 31) break;
            rotl_bits(K, 10, 61);
            uint8_t top = (K[0] >> 4) & 0xF; top = sbox[top];
            K[0] = (uint8_t)((top << 4) | (K[0] & 0x0F));
            int rc = r + 1;
            K[7] ^= (uint8_t)((rc >> 1) & 0x0F);
            K[8] ^= (uint8_t)((rc & 1) << 7);
        }
    }

    static inline void KeySchedule128(const uint8_t* key, uint8_t* rk) {
        uint64_t K0 = load64_be(key), K1 = load64_be(key + 8);

        for (int r = 0; r < 32; ++r) {

            store64_be(rk + 8 * r, K0);
            if (r == 31) break;

            uint64_t nK0 = (K0 << 61) | (K1 >> 3);
            uint64_t nK1 = (K1 << 61) | (K0 >> 3);
            K0 = nK0; K1 = nK1;

            uint8_t msb = (uint8_t)(K0 >> 56);
            uint8_t hi = sbox[msb >> 4], lo = sbox[msb & 0xF];
            K0 = (K0 & 0x00FFFFFFFFFFFFFFULL) | (uint64_t)((hi << 4) | lo) << 56;

            uint8_t rc = (uint8_t)(r + 1);
            K0 ^= (uint64_t)((rc >> 2) & 0x07);         // bits 2..0
            K1 ^= (uint64_t)(rc & 0x03) << 62;        // bits 63..62
        }
    }

    static inline void KeyExpansion(const uint8_t* key, int keybits, uint8_t* rk) {
        if (keybits == 80) KeySchedule80(key, rk);
        else            KeySchedule128(key, rk);
    }

public:
    static void EncryptBlock(uint8_t out[8], const uint8_t in[8], const uint8_t* key, int keysize, int breakpoint = 4, int breakN = 0) {
        
        int b1N = 0, b2N = 0, b3N = 0; 
        
        uint64_t s = load64_be(in);
        
        if(breakpoint == 0) goto encrexit;
        
        uint8_t rk[32 * 8]; KeyExpansion(key, keysize*8, rk);
        for (int r = 0; r < 31; ++r) {
            s ^= load64_be(rk + 8 * r);
            if(breakpoint == 1 && ++b1N == breakN) goto encrexit; // breakpoint 1
            s = SBoxLayer(s);
            if(breakpoint == 2 && ++b2N == breakN) goto encrexit; // breakpoint 2
            s = PLayer(s);
            if(breakpoint == 3 && ++b3N == breakN) goto encrexit; // breakpoint 3
        }
        s ^= load64_be(rk + 31 * 8);
        // breakpoint 1
        // breakpoint 4
        
    encrexit:
        store64_be(out, s);
        
    }

    static void DecryptBlock(uint8_t out[8], const uint8_t in[8], const uint8_t* key, int keysize, int breakpoint = 4, int breakN = 0) {
        
        int b1N = 0, b2N = 0, b3N = 0;
        
        uint64_t s = load64_be(in);
        
        if(breakpoint == 0) goto decrexit;
        
        uint8_t rk[32 * 8]; KeyExpansion(key, keysize*8, rk);
        s ^= load64_be(rk + 31 * 8);
        if(breakpoint == 1 && ++b1N == breakN) goto decrexit; // breakpoint 1
        for (int r = 30; r >= 0; --r) {
            s = InvPLayer(s);
            if(breakpoint == 2 && ++b2N == breakN) goto decrexit; // breakpoint 2
            s = InvSBoxLayer(s);
            if(breakpoint == 3 && ++b3N == breakN) goto decrexit; // breakpoint 3
            s ^= load64_be(rk + 8 * r);
            if(breakpoint == 1 && ++b1N == breakN) goto decrexit; // breakpoint 1
        }
        // breakpoint 4
        
    decrexit:
        store64_be(out, s);
        
    }
};

// ---- S-box tables ----
const uint8_t TPRESENT::sbox[16] = { 0xC,0x5,0x6,0xB,0x9,0x0,0xA,0xD,0x3,0xE,0xF,0x8,0x4,0x7,0x1,0x2 };
const uint8_t TPRESENT::inv_sbox[16] = { 0x5,0xE,0xF,0x8,0xC,0x1,0x2,0xD,0xB,0x4,0x6,0x3,0x0,0x7,0x9,0xA };


#endif // TPRESENT_H

