/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include <functional>
#include <iosfwd>
#include <tuple>

#include "common/bit_util.h"
#include "common/common_types.h"
#include "common/fp/fpcr.h"
#include "frontend/ir/location_descriptor.h"

namespace Dynarmic::A64 {

/**
 * LocationDescriptor describes the location of a basic block.
 * The location is not solely based on the PC because other flags influence the way
 * instructions should be translated.
 */
class LocationDescriptor {
public:
    static constexpr u64 PC_MASK = 0x00FF'FFFF'FFFF'FFFFull;
    static constexpr u32 FPCR_MASK = 0x07C8'0000;

    LocationDescriptor(u64 pc, FP::FPCR fpcr) : pc(pc & PC_MASK), fpcr(fpcr.Value() & FPCR_MASK) {}

    explicit LocationDescriptor(const IR::LocationDescriptor& o)
        : pc(o.Value() & PC_MASK), fpcr((o.Value() >> 37) & FPCR_MASK) {}

    u64 PC() const { return Common::SignExtend<56>(pc); }
    FP::FPCR FPCR() const { return fpcr; }

    bool operator == (const LocationDescriptor& o) const {
        return std::tie(pc, fpcr) == std::tie(o.pc, o.fpcr);
    }

    bool operator != (const LocationDescriptor& o) const {
        return !operator==(o);
    }

    LocationDescriptor SetPC(u64 new_pc) const {
        return LocationDescriptor(new_pc, fpcr);
    }

    LocationDescriptor AdvancePC(int amount) const {
        return LocationDescriptor(static_cast<u64>(pc + amount), fpcr);
    }

    u64 UniqueHash() const noexcept {
        // This value MUST BE UNIQUE.
        // This calculation has to match up with EmitTerminalPopRSBHint
        const u64 fpcr_u64 = static_cast<u64>(fpcr.Value()) << 37;
        return pc | fpcr_u64;
    }

    operator IR::LocationDescriptor() const {
        return IR::LocationDescriptor{UniqueHash()};
    }

private:
    u64 pc;        ///< Current program counter value.
    FP::FPCR fpcr; ///< Floating point control register.
};

/**
 * Provides a string representation of a LocationDescriptor.
 *
 * @param o          Output stream
 * @param descriptor The descriptor to get a string representation of
 */
std::ostream& operator<<(std::ostream& o, const LocationDescriptor& descriptor);

} // namespace Dynarmic::A64

namespace std {
template <>
struct less<Dynarmic::A64::LocationDescriptor> {
    bool operator()(const Dynarmic::A64::LocationDescriptor& x, const Dynarmic::A64::LocationDescriptor& y) const noexcept {
        return x.UniqueHash() < y.UniqueHash();
    }
};
template <>
struct hash<Dynarmic::A64::LocationDescriptor> {
    size_t operator()(const Dynarmic::A64::LocationDescriptor& x) const noexcept {
        return std::hash<u64>()(x.UniqueHash());
    }
};
} // namespace std
