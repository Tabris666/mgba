/* Copyright (c) 2013-2016 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef GB_H
#define GB_H

#include <mgba-util/common.h>

CXX_GUARD_START

#include <mgba/core/cpu.h>
#include <mgba/core/interface.h>
#include <mgba/core/log.h>
#include <mgba/core/timing.h>

#include <mgba/internal/gb/audio.h>
#include <mgba/internal/gb/memory.h>
#include <mgba/internal/gb/sio.h>
#include <mgba/internal/gb/timer.h>
#include <mgba/internal/gb/video.h>

extern const uint32_t DMG_LR35902_FREQUENCY;
extern const uint32_t CGB_LR35902_FREQUENCY;
extern const uint32_t SGB_LR35902_FREQUENCY;

mLOG_DECLARE_CATEGORY(GB);

// TODO: Prefix GBAIRQ
enum GBIRQ {
	GB_IRQ_VBLANK = 0x0,
	GB_IRQ_LCDSTAT = 0x1,
	GB_IRQ_TIMER = 0x2,
	GB_IRQ_SIO = 0x3,
	GB_IRQ_KEYPAD = 0x4,
};

enum GBIRQVector {
	GB_VECTOR_VBLANK = 0x40,
	GB_VECTOR_LCDSTAT = 0x48,
	GB_VECTOR_TIMER = 0x50,
	GB_VECTOR_SIO = 0x58,
	GB_VECTOR_KEYPAD = 0x60,
};

struct LR35902Core;
struct mCoreSync;
struct mAVStream;
struct GB {
	struct mCPUComponent d;

	struct LR35902Core* cpu;
	struct GBMemory memory;
	struct GBVideo video;
	struct GBTimer timer;
	struct GBAudio audio;
	struct GBSIO sio;
	enum GBModel model;

	struct mCoreSync* sync;
	struct mTiming timing;

	uint8_t* keySource;

	bool isPristine;
	size_t pristineRomSize;
	size_t yankedRomSize;
	uint32_t romCrc32;
	struct VFile* romVf;
	struct VFile* biosVf;
	struct VFile* sramVf;
	struct VFile* sramRealVf;
	uint32_t sramSize;
	int sramDirty;
	int32_t sramDirtAge;
	bool sramMaskWriteback;

	struct mCoreCallbacksList coreCallbacks;
	struct mAVStream* stream;

	bool cpuBlocked;
	bool earlyExit;
	struct mTimingEvent eiPending;
	unsigned doubleSpeed;
};

struct GBCartridge {
	uint8_t entry[4];
	uint8_t logo[48];
	union {
		char titleLong[16];
		struct {
			char titleShort[11];
			char maker[4];
			uint8_t cgb;
		};
	};
	char licensee[2];
	uint8_t sgb;
	uint8_t type;
	uint8_t romSize;
	uint8_t ramSize;
	uint8_t region;
	uint8_t oldLicensee;
	uint8_t version;
	uint8_t headerChecksum;
	uint16_t globalChecksum;
	// And ROM data...
};

void GBCreate(struct GB* gb);
void GBDestroy(struct GB* gb);

void GBReset(struct LR35902Core* cpu);
void GBDetectModel(struct GB* gb);

void GBUpdateIRQs(struct GB* gb);
void GBHalt(struct LR35902Core* cpu);

struct VFile;
bool GBLoadROM(struct GB* gb, struct VFile* vf);
bool GBLoadSave(struct GB* gb, struct VFile* vf);
void GBUnloadROM(struct GB* gb);
void GBSynthesizeROM(struct VFile* vf);

bool GBIsBIOS(struct VFile* vf);
void GBLoadBIOS(struct GB* gb, struct VFile* vf);

void GBSramClean(struct GB* gb, uint32_t frameCount);
void GBResizeSram(struct GB* gb, size_t size);
void GBSavedataMask(struct GB* gb, struct VFile* vf, bool writeback);
void GBSavedataUnmask(struct GB* gb);

struct Patch;
void GBApplyPatch(struct GB* gb, struct Patch* patch);

bool GBIsROM(struct VFile* vf);
void GBGetGameTitle(const struct GB* gba, char* out);
void GBGetGameCode(const struct GB* gba, char* out);

void GBTestKeypadIRQ(struct GB* gb);

void GBFrameEnded(struct GB* gb);

CXX_GUARD_END

#endif
