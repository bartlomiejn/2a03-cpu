#ifndef INC_2A03_OAMDMA_H
#define INC_2A03_OAMDMA_H

#include <cstdint>
#include <functional>

namespace NES {

/// Sprite DMA Controller
/// https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/dma.md
class OAMDMA {
   public:
    OAMDMA() = default;

    /// Begin OAM DMA read/write cycle
    void write_oamdma(uint8_t _oamdma);

    std::function<void(uint8_t)> dma_schedule_cpu;  ///< Schedule DMA transfer
    std::function<void(uint16_t)> dma_read;         ///< Perform single DMA read
    std::function<void(uint16_t)> dma_write;  ///< Perform single DMA write

   protected:
    uint8_t oamdma = 0x0;
};

}  // namespace NES

#endif
