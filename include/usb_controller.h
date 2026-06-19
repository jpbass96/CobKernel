#ifndef _usb_controller_h
#define _usb_controller_h
#include "types.h"

//Host Controller capability Registers
#define CAPLENGTH_OFF 0x0
#define CAPLENGTH_MASK 0xFF
#define HCSI_VER_MASK 0xFFFF00



#define HCSPARAMS1 0x4
#define HCSPARAMS1_MAXSLOTS_MASK 0xFF
#define HCSPARAMS1_MAXSLOTS_LSB 0
#define HCSPARAMS1_MAXINTRS_MASK 0x7FF00
#define HCSPARAMS1_MAXINTRS_LSB 8
#define HCSPARAMS1_MAXPORTS_MASK 0xFF000000
#define HCSPARAMS1_MAXPORTS_LSB 24

#define HCSPARAMS2 0x8
#define HCSPARAMS2_ERSTMAX_MASK 0xF0
#define HCSPARAMS2_ERSTMAX_LSB 4 //maximum event segment table size supported by controller.
#define HCSPARAMS2_MSPB_LO_LSB 27 //max scratchpad buffers lower 5 bits
#define HCSPARAMS2_MSPB_LO_MASK 0xF8000000
#define HCSPARAMS2_MSPB_HI_LSB 21 //max scratchpad buffers lower 5 bits
#define HCSPARAMS2_MSPB_HI_MASK 0x3E00000
#define HCSPARAMS2_MSPB_GET_BITS(val)  ((((val) & HCSPARAMS2_MSPB_HI_MASK) >> (HCSPARAMS2_MSPB_HI_LSB-5)) | (((val) & HCSPARAMS2_MSPB_LO_MASK) >> (HCSPARAMS2_MSPB_LO_LSB)))

#define HCSPARAMS3 0xc
#define HCCPARAMS1 0x10
#define HCCPARAMS1_CSZ_MASK 0x4
#define HCCPARAMS1_CSZ_LSB 2
#define DBOFF 0x14
#define RTSOFF 0x18
#define HCCPARAMS2 0x1c

//Operational Registers
#define USBCMD 0x0
#define USBCMD_RUNSTOP_LSB 0
#define USBMCMD_RUNSTOP_MASK 0x1
#define USBMCMD_HCRST_LSB 1
#define USBMCMD_HCRST_MASK 2


#define USBSTS 0x4
#define USBSTS_HCH_MASK 0x1
#define USBSTS_CNR_LSB 11
#define USBSTS_CNR_MASK 0x800

#define PAGESIZE 0x8
#define CNCTRL 0x14

//XHCI spec table 5-24
//Command Ring Control Register
#define CRCR 0x18
#define CRCR_RCS_LSB 0 //ring cycle state
#define CRCR_RCS_MASK 0x1
#define CRCR_CS_LSB 1 //command stop bit
#define CRCR_CS_MASK 0x2
#define CRCR_CA_LSB 2 //command abort bit
#define CRCR_CA_MASK 0x4
#define CRCR_CRR_LSB 3 //command ring running status
#define CRCR_CRR_MASK 0x8
#define CRCR_CRPTR_LSB 6 //command ring pointer (lower bits)
#define CRCR_CRPTR_MASK 0xFFFFFFFFFFFFFFC0


#define DCBAAP 0x30
#define CONFIG 0x38
#define PORT_REG_START 0x400
#define PORT_REG(n) (PORT_REG_START + 0x10 * (n-1))
#define PORTSC(n) (PORT_REG(n) + 0x0)

//runtime registers
#define XHCI_IR0 0x20
#define INTERRUPTER_REG_SIZE 32
#define XHCI_IR(N) (XHCI_IR0 + INTERRUPTER_REG_SIZE*(N))
#define XHCI_IMAN(N) XHCI_IR(N)
#define XHCI_IR_ERSTSZ(N) (XHCI_IR(N) + 0x08) // Event ring segment table size
#define XHCI_IR_ERSTZ_MASK 0xFF
#define XHCI_IR_ERSTBA(N) (XHCI_IR(N) + 0x10) // Event ring segment table base address
#define XHCI_IR_ERDP(N) (XHCI_IR(N) + 0x18) //event ring dequeue pointer


//Transfer Ring: a set of transfer ring segments pointed to by enqueue/dequeue pointers
//Transfer Ring Segmnet: a contiguous set of TRBs in a transfer ring. One ring can have multiple segments
//                       connectedy by a link TRB
//TRB: transfer buffer that goes in a trb ring
//Command ring: special ring reserved for issuing commands to the XHCI controller for device management
//Transfer ring: main ring for initiating transfers to/from device endpoints
//Event ring: used by devices to signal events
//cycle bit: producer and consumer maintain a copy of the cycle bit. You "own" a TRB if your internally maintained
//           cycle bit matches the TRB cycle bit. When you take the TRB you flip the cycle bit. Link TRBs can have a
//           toggle cycle bit flag which causes the PCS or CCS flag to flip
#define XHCI_MIN_CONTEXT_STRUCT_SIZE 0x400
#define XHCI_CONTEXT_ENTRY_SIZE 8
#define XCHI_CONTEXT_ALIGN 64
#define XHCI_TRB_ALIGN 16
#define XHCI_TRB_TRANSFER_SEGMENT_ALIGN 16
#define XHCI_TRB_COMMAND_SEGMENT_ALIGN 64
#define XHCI_ERS_TABLE_ALIGN 64
#define XHCI_TRB_RING_ALIGN 65536
#define XHCI_MIN_PGSIZE 4096

//#define XHCI_SET_BITS(field, val) ((val << field##_LSB) & field##_MASK )

#define TRB_CTRL_IOC 5
#define TRB_CTRL_IDT_LSB 6
#define TRB_CTRL_IDT_BITS(val) (((val) << TRB_CTRL_IDT_LSB) & 1)
#define TRB_CTRL_TRB_TYPE_LSB 10
#define TRB_CTRL_TRB_TYPE_MASK  0xFC00
#define TRB_CTRL_C_BITS(val) ((val) & 1) //cycle bit
#define TRB_CTRL_C_LSB 0
#define TRB_CTRL_C_MASK 0x1
#define TRB_CTRL_TC_LSB 1
#define TRB_CTRL_TC_BITS(val) ((((val) & 1 ) << TRB_CTRL_TC_LSB))//toggle cycle bit
#define TRB_CTRL_TYPE_BITS(type) (((type) << TRB_CTRL_TRB_TYPE_LSB) & TRB_CTRL_TRB_TYPE_MASK)


#define TRB_STATUS_CC_LSB 24
#define TRB_STATUS_CC_MASK 0xFF000000
#define TRB_STATUS_CC_GET_BITS(val) ((val) & TRB_STATUS_CC_MASK >> TRB_STATUS_CC_LSB)

//transfer ring TRBs
#define TRB_TYPE_LINK 6
#define TRB_TYPE_NOOP 8
#define TRB_TYPE_ENABLE_SLOT 9

//Command ring TRBs
#define TRB_TYPE_CMD_NOOP 23 

#define TRB_RING_MIN_SEGMENT_SIZE 16

//TRB completion codes
#define TRB_INVALID 0
#define TRB_SUCCESS 1

//Doorbell Targets
#define DB_TGT_CMD 0

#define SCRATCHPAD_BUFFER_ENTRY_SIZE 8
struct __attribute__((packed, aligned(XHCI_TRB_ALIGN))) xhci_ers_entry {
    u64 ring_segment_base;
    u32 ring_segment_size:16;
    u32 rsvd:16;
    u32 rsvd2;
};


struct  __attribute__((packed)) xhci_ers_table {
    struct xhci_ers_entry *head;
    u32 entries;
};

struct __attribute__((packed, aligned(XHCI_TRB_ALIGN))) xhci_trb {
    u64 data_ptr;
    u32 status;
    u32 control;
};


struct xhci_trb_ring {
    struct xhci_trb *head;
    struct xhci_trb *tail;
    struct xhci_trb *enq_nxt;
    struct xhci_trb *dq_nxt;
    ////producer or consumer cycle state.  Software maintains  PCS for submission queues, and CCS for completion queues.
    u8 cs; 
    
};

struct __attribute__((packed, aligned(XCHI_CONTEXT_ALIGN))) xhci_slot_context  {
    //word 0
    u32 route_str : 20;
    u32 speed : 4;
    u32 rsdvz0 : 1;
    u32 mtt : 1;
    u32 hub : 1;
    u32 context_entries: 5;

    //word 1
    u32 max_exit_latency : 16;
    u32 root_hub_port_num : 8;
    u32 num_ports : 8;

    //word 2
    u32 tt_slot_id : 8;
    u32 tt_port_num : 8;
    u32 ttt : 2;
    u32 rsdvz1 : 4;
    u32 interrupter_tgt :  10;

    //word 3 
    u32 usb_dev_addr : 8;
    u32 rsdvz2 : 19;
    u32 slot_state : 5;

    //words4-8 
    u32 rsdvz3[4];

} ;
 
struct usb_ctrl_ops {


};

struct usb_ctrl {
    void *mmio_base;
    uintptr operational_reg_base;
    uintptr runtime_reg_base;
    uintptr doorbell_array_base;
    void *device_context_arr;
    struct xhci_trb_ring *cmdring;
    struct xhci_trb_ring *primary_event_ring;
    struct xhci_ers_table *ers_table;
    struct usb_ctrl_ops ops;
    u8 max_ports;
    u8 max_devices;
    u16 max_intrs;
    u32 max_erst;
    u32 pgsize;
};

struct device *xhci_probe(void *base_addr);
    
#endif