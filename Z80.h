#ifndef __Z80_INCLUDED__
#define __Z80_INCLUDED__

#include "Z80emu.h"
#include "z80user.h"
#include "z80config.h"

/*  Memory Map  */

#define WORKRAM   0x02000

#define state   (&(m_context.state))

class Z80
{
public:

    Z80();
    virtual ~Z80();

    bool m_giveupz80;
    bool in_r_exec;
    bool ppt_running;
    long m_mtPLevel;
    bool m_mtutorBoot;
    bool m_MtTrace;
    unsigned short  m_zclock;      // one second clock for mtutor
    
    ZEXTEST m_context;

    int	emulate(int opcode,
        int elapsed_cycles, int number_cycles);

    int Z80Emulate(int number_cycles);

    int Z80Interrupt(int data_on_bus);

    int Z80NonMaskableInterrupt();

    void Z80Reset(void);

    bool Z80BreakPoint (int pc, bool step);

    void PatchL2 (void);
    void PatchL3 (void);
    void PatchL4 (void);
    void PatchL5 (void);
    void PatchL6 (void);
    void PatchColor (unsigned char low_getvar);

    virtual unsigned char inputZ80(unsigned char data);

    virtual void outputZ80(unsigned char data, unsigned char acc);

    virtual int check_pcZ80(void);

    unsigned char ReadRAM(unsigned short offset) const
    {
        return m_context.memory[offset];
    }
    unsigned short ReadRAMW(unsigned short offset) const
    {
        return m_context.memory[offset] | (m_context.memory[offset + 1] << 8);
    }

    void WriteRAM(unsigned short offset, unsigned char data)
    {
#ifdef Z80_HANDLE_SELF_MODIFYING_CODE
        m_context.memory[(offset)] = data;
#else
        if ((offset) >= WORKRAM)
        {
            m_context.memory[(offset)] = data;
        }
        else
        {
            printf("!!! WriteRAM Attempted write at offset %04x with data "
                "%02x failed.  PC = %04x\n", offset, data, state->pc);
        }
#endif
    }

    void WriteRAMW(unsigned short offset, unsigned short data)
    {
#ifdef Z80_HANDLE_SELF_MODIFYING_CODE
        m_context.memory[(offset)] = data & 0xff;
        m_context.memory[(offset + 1)] = (data << 8) & 0xff;
#else
        if ((offset) >= WORKRAM)
        {
            m_context.memory[(offset)] = data & 0xff;
            m_context.memory[(offset + 1)] = (data >> 8) & 0xff;
        }
        else
        {
            printf("!!! WriteRAMW Attempted write at offset %04x with data "
                "%02x failed.  PC = %04x\n", offset, data, state->pc);
        }
#endif
    }

};
#endif
