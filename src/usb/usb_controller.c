#include "usb_controller.h"
#include "malloc.h"
#include "device.h"
#include "string.h"
#include "util.h"
#include "types.h"
#include "time.h"

static inline u32 xhci_read32(struct usb_ctrl *dev, uintptr offset) {
    LOG_DEBUG("READING DWC REG AT 0x%lx\n\r", (uintptr)dev->mmio_base + offset);
    return read32((uintptr)dev->mmio_base + offset);
}

static inline void xhci_write32(struct usb_ctrl *dev, uintptr offset, u32 data) {
    write32((uintptr)dev->mmio_base + offset, data);
}

#define read_opreg_poll(ctrl, val, offset,  condition, sleep_us, timeout_us) read32_poll_timeout(val, (uintptr)(ctrl->mmio_base + ctrl->operational_reg_base + offset), condition, sleep_us, timeout_us)

static inline u32 read_opreg(struct usb_ctrl *dev, uintptr offset) {
    LOG_DEBUG("READING DWC REG AT 0x%lx\n\r", (uintptr)dev->mmio_base + dev->operational_reg_base + offset);
    return read32((uintptr)dev->mmio_base + dev->operational_reg_base + offset);
}

static inline void write_opreg(struct usb_ctrl *dev, uintptr offset, u32 data) {
    write32((uintptr)dev->mmio_base + dev->operational_reg_base + offset, data);
}
static inline u32 read_dbreg(struct usb_ctrl *dev, uintptr offset) {
    return read32((uintptr)dev->mmio_base + dev->doorbell_array_base + offset);
}

static inline void write_db_reg(struct usb_ctrl *dev, uintptr offset, u32 data) {
    write32((uintptr)dev->mmio_base + dev->doorbell_array_base + offset, data);
}

static inline u32 read_runtime_reg(struct usb_ctrl *dev, uintptr offset) {
     return read32((uintptr)dev->mmio_base + dev->runtime_reg_base + offset);
}

static inline void write_runtime_reg(struct usb_ctrl *dev, uintptr offset, u32 data) {
    write32((uintptr)dev->mmio_base + dev->runtime_reg_base + offset, data);
}




static struct usb_ctrl_ops xhci_ops = {

};

int wait_xhci_rdy(struct usb_ctrl *ctrl) {
    u32 reg;
    int timeout;
    timeout = read_opreg_poll(ctrl, reg, USBSTS, !(reg & USBSTS_CNR_MASK), 500, 50000);
    return timeout;
}

static inline int halt_xhci_ctrl(struct usb_ctrl *ctrl) {
    u32 reg;
    int timeout;
    reg = read_opreg(ctrl, USBCMD);
    //clear RS bit to halt controller
    write_opreg(ctrl, USBCMD, reg & ~(USBMCMD_RUNSTOP_MASK));
 
    //wait for halted bit to go high
    timeout = read_opreg_poll(ctrl, reg, USBSTS, (reg & USBSTS_HCH_MASK) == USBSTS_HCH_MASK, 500, 50000);
    
    return timeout;
}

static inline int enable_xhci_ctrl(struct usb_ctrl *ctrl) {
    u32 reg;
    int timeout;
    reg = read_opreg(ctrl, USBCMD);
    //clear RS bit to halt controller
    write_opreg(ctrl, USBCMD, reg | (USBMCMD_RUNSTOP_MASK));

    timeout = read_opreg_poll(ctrl, reg, USBSTS, (reg & USBSTS_HCH_MASK) == 0, 500, 50000);

    if (timeout) {
        LOG_ERROR("Timed out enabling xhci ctrln\n\r");
        return -1;
    }
    return 0;
}

int reset_xhci_ctrl(struct usb_ctrl *ctrl) {
    int timeout;
    u32 reg;
    write_opreg(ctrl, USBCMD, USBMCMD_HCRST_MASK);
    timeout = read_opreg_poll(ctrl, reg, USBCMD, (reg & USBMCMD_HCRST_MASK) == 0, 500, 50000);;
    if (timeout) {
        LOG_ERROR("Timeout resetting xhci controller\n\r");
        return -1;
    }
    return 0;
 }

struct xhci_trb_ring *allocate_trb_ring(struct usb_ctrl *ctrl, u32 entries, u32 align) {
    struct xhci_trb_ring *ring;
    void *mem;
    size_t bytes;
    u64 trb_bytes;

    trb_bytes = sizeof(struct xhci_trb)*entries;
    bytes = sizeof(struct xhci_trb_ring) + trb_bytes;
    
    mem = kmalloc_aligned(bytes, align);

    if (mem == NULL) {
        return NULL;
    }

    //to keep TRB alignment easy, we will place the TRBs at the beginning of the region, and the ring structure itself at the end
    ring = (struct xhci_trb_ring*)(mem + sizeof(struct xhci_trb)*entries);
    ring->head = (struct xhci_trb *)mem;
    //initialize whole ring to 0
    LOG_INFO("Initializing %lx bytes to 0 at ring head\n\r", trb_bytes);
    memset((void*)ring->head, 0, trb_bytes);

    ring->dq_nxt = ring->head;
    ring->enq_nxt = ring->head;
    ring->tail = ring->head + (entries-1);
    ring->tail->data_ptr = (u64)ring->head;

    //set TC bit so PCS/CCS toggle when reaching link TRB
    ring->tail->control = TRB_CTRL_TYPE_BITS(TRB_TYPE_LINK) | TRB_CTRL_TC_BITS(1); 
    //ring->tail->control = XHCI_SET_BITS(TRB_CTRL_TYPE, TRB_TYPE_LINK) | XHCI_SET_BITS(TRB_CTRL_TC, 1);

    //initialize PCS to 1 as per XHCI spec 4.9.2.2
    ring->cs = 1;
    return ring;
}



void free_trb_ring(struct xhci_trb_ring *ring) {
    //we made the first entry the tip of the malloc, so call free on that
    kfree((void*)ring->head);
}

struct xhci_ers_table *allocate_ers_table(struct usb_ctrl *ctrl, u32 num_entries) {
    struct xhci_ers_table *ers_table;
    u64 bytes, ers_entry_bytes;
    void *mem;

    ers_entry_bytes = sizeof(struct xhci_ers_entry)*num_entries;
    bytes = ers_entry_bytes + sizeof(struct xhci_ers_table);
    mem = kmalloc_aligned(bytes,  XHCI_ERS_TABLE_ALIGN);
    
    if (mem == NULL) {
        LOG_ERROR("Could not allocate ERS table\n\r");
        return NULL;
    }

    memset(mem, 0, bytes);
    ers_table = mem + sizeof(struct xhci_ers_entry)*num_entries;
    ers_table->head = (struct xhci_ers_entry *) mem;
    ers_table->entries = num_entries;

  
    return ers_table;
}

//assumes that the device context base array is already initialized
int allocate_scratchpad_buffs(struct usb_ctrl *ctrl) {
    u32 reg;
    u64 *scratchpad_buffer_array ;
    size_t scratchpad_buff_size;
    void *mem;

    if (ctrl->device_context_arr == NULL) {
        return -1;
    }

    reg = xhci_read32(ctrl, HCSPARAMS2);
    reg = HCSPARAMS2_MSPB_GET_BITS(reg);
    //allocate enough space for all scratchpad buffers + each 8 byte address in the scratchpad base address table
    scratchpad_buff_size = ctrl->pgsize * reg;
    mem = kmalloc_aligned(scratchpad_buff_size + (reg*SCRATCHPAD_BUFFER_ENTRY_SIZE), ctrl->pgsize);
    LOG_INFO("Scratchpad mem at 0x%lx with %d entries of size %d\n\r", mem, reg, ctrl->pgsize);
    //set first device context entry to scratchpad buffer array.
    *(u64*)ctrl->device_context_arr = (u64)mem + scratchpad_buff_size; 
    
    //zeroize scratchpad area as per xhci spec section 4.20
    memset(mem, 0, scratchpad_buff_size);

    if (mem == NULL) {
        LOG_ERROR("Could not allocate scratchpad buffer\n\r");
        return -1;
    }

    scratchpad_buffer_array = *(u64**)ctrl->device_context_arr;
    for (u32 i = 0; i < reg; i++) {
         LOG_INFO("Setting entry 0x%lx to %lx\n\r", scratchpad_buffer_array, (u64)mem + i*ctrl->pgsize);
        *scratchpad_buffer_array++ = (u64)mem + i*ctrl->pgsize;
       
    }


    LOG_INFO("Allocated %d scratchpad buffers of size %d\n\r", reg, ctrl->pgsize);
   
    return 0;
}

int allocate_device_ctx_arr(struct usb_ctrl *ctrl) {
    u64 reg;
    u64 *entry;
    u64 numbytes;
    u64 context_struct_size;

    ctrl->pgsize = read_opreg(ctrl, PAGESIZE) << 12;

    reg = xhci_read32(ctrl, HCCPARAMS1);
    //double entry size if 64 byte context data structures used
    context_struct_size = XHCI_MIN_CONTEXT_STRUCT_SIZE << get_bits_mask(reg, HCCPARAMS1_CSZ_MASK, HCCPARAMS1_CSZ_LSB);
    LOG_INFO("pgsize is 0x%x, HCCPARAMS1 is 0x%lx\n\r", ctrl->pgsize, reg);

    //Allocate dcbaap as per XHCI spec section 6.1
    //allocate data for each entry up front
    //allocate extra data to align context structs
    //align to minimum 4096 byte page size to avoid page boundary crossings
    numbytes = (ctrl->max_devices + 1) * XHCI_CONTEXT_ENTRY_SIZE + ((ctrl->max_devices)*context_struct_size) + (XHCI_MIN_PGSIZE-1);
    ctrl->device_context_arr = kmalloc_aligned(numbytes, XHCI_MIN_PGSIZE);

    if (ctrl->device_context_arr == NULL) {
        return -1;
    }

   
    if (allocate_scratchpad_buffs(ctrl)) {
        LOG_ERROR("Could not allocate scratchpad bufs\n\r");
        kfree(ctrl->device_context_arr);
        return -1;
    }

    
    LOG_INFO("context_arr at 0x%lx of %ld bytes\n\r ", (u64)ctrl->device_context_arr, numbytes);
    if (ctrl->device_context_arr == NULL)
        return -1;
    
    write_opreg(ctrl, DCBAAP, (u64)ctrl->device_context_arr);
    write_opreg(ctrl, DCBAAP+4, (u64)ctrl->device_context_arr >> 32);
    entry = (u64*)ctrl->device_context_arr;
    LOG_INFO("num devices is %d\n\r",  ctrl->max_devices);
    //initialize device context array
    
    *++entry = alignup((u64)ctrl->device_context_arr +  (ctrl->max_devices + 1) * XHCI_CONTEXT_ENTRY_SIZE, XHCI_MIN_PGSIZE);
 
    for (u8 i = 1; i < ctrl->max_devices; i++, entry++) {
        *entry = *(entry-1) + context_struct_size;
    }
    

    reg = read_opreg(ctrl, DCBAAP);
    reg = reg | ((u64)read_opreg(ctrl, DCBAAP+4) << 32);
    ctrl->device_context_arr = (void*)reg;
   
    return 0;
}

void free_device_ctx_arr(struct usb_ctrl *ctrl) {
    kfree(*(u64**)ctrl->device_context_arr); //free scratchpad buffer array
    kfree(ctrl->device_context_arr); //free rest of device context array
}

void dump_ctrl_regs(struct usb_ctrl *ctrl) {
    u64 offset;
    offset = 0;
    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset +=4; 

    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset +=4; 

    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset  = 0x14; 
 
    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset +=4; 

    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset +=4; 
    
    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset = 0x30; 

    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset +=4;
    
    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset +=4; 

    LOG_INFO("opreg offset 0x%lx: 0x%x\n\r", offset, read_opreg(ctrl, offset));
    offset +=4; 

    offset = 0x20;
    LOG_INFO("interrupt register offset 0x%lx: 0x%x\n\r", offset,  read_runtime_reg(ctrl, offset));

    offset = 0x24;
    LOG_INFO("interrupt register offset 0x%lx: 0x%x\n\r", offset, read_runtime_reg(ctrl, offset));

    offset = 0x28;
    LOG_INFO("interrupt register offset 0x%lx: 0x%x\n\r", offset,  read_runtime_reg(ctrl, offset));

    offset = 0x30;
    LOG_INFO("interrupt register offset 0x%lx: 0x%x\n\r", offset,  read_runtime_reg(ctrl, offset));

    offset = 0x34;
    LOG_INFO("interrupt register offset 0x%lx: 0x%x\n\r", offset,  read_runtime_reg(ctrl, offset));

    offset = 0x38;
    LOG_INFO("interrupt register offset 0x%lx: 0x%x\n\r", offset,  read_runtime_reg(ctrl, offset));
    offset = 0x3c;
    LOG_INFO("interrupt register offset 0x%lx: 0x%x\n\r", offset,   read_runtime_reg(ctrl, offset));

}


void dump_trb_ring(struct xhci_trb *head, u32 entries) {
    struct xhci_trb *cur;

    cur = head;
    for (int i = 0; i < entries; i++, cur++) {
        u32 *addr = (u32*)(cur);
        LOG_INFO("  TRB at addr 0x%lx\n\r", (uintptr)addr);
        for (int j= 0 ; j < 4; j++, addr++) {
            LOG_INFO("    0x%lx\n\r", *addr);
        }
    }
}

void dump_ers_table (struct usb_ctrl *ctrl) {

    u64 reg, size;
    size =    read_runtime_reg(ctrl, XHCI_IR_ERSTSZ(0));

    LOG_INFO("  ERS table has %ld entries\n\r", size);
    reg = (u64)read_runtime_reg(ctrl, XHCI_IR_ERSTBA(0)) | ((u64)read_runtime_reg(ctrl, XHCI_IR_ERSTBA(0)+4) << 32);
    struct xhci_ers_entry *cur = (struct xhci_ers_entry *)reg;
    struct xhci_trb *trb;
    for (int i = 0; i < 1; i++) {
        LOG_INFO("entry %d at addr %lx has segment size %d and base 0x%lx\n\r\n\r", i, (u64)cur, cur->ring_segment_size, cur->ring_segment_base);
        trb = (struct xhci_trb *)cur->ring_segment_base;
        dump_trb_ring(trb, cur->ring_segment_size);
    }
}

void dump_all_tables(struct usb_ctrl *ctrl) {

    LOG_INFO("-----------------------Command ring table dump--------------------------\n\r");
    void *crcr;
    crcr = ctrl->cmdring->head;
    LOG_INFO("Command ring base pointer at 0x%lx\n\r", (u64)crcr);

    dump_trb_ring(ctrl->cmdring->head, 16);

    LOG_INFO("-----------------------Command ring table dump--------------------------\n\r");


    LOG_INFO("-----------------------ERS Segment ring table dump--------------------------\n\r");
    dump_ers_table(ctrl);

    LOG_INFO("-----------------------ERS Segment ring table dump--------------------------\n\r");
}

//update_cs argument tells function to update rings cycle state. Speculative checks
//of the next trb should not set this flag. For now assuming tail is always a link TRB with the
//toggle cycle bit set.
static inline struct xhci_trb *get_next_trb(struct xhci_trb_ring *ring, struct xhci_trb *cur, u8 update_cs) {
    //skip link TRB
    if (cur == (ring->tail - 1)) {
        if (update_cs) {
            ring->cs = !ring->cs;
        }
        return ring->head;
    }

    return cur+1;
}

int poll_event_queue(struct usb_ctrl *ctrl, struct xhci_trb_ring *ring) {
    struct xhci_trb *event_trb;
    event_trb = ring->dq_nxt;


    u32 timeout = 0;
    LOG_INFO("waiting on event trb at 0x%lx\n\r", event_trb);
    LOG_INFO("waiting for ccs value %d\n\r", ring->cs);
    LOG_INFO("current TRB control value: %d\n\r", event_trb->control);
    for (;;) {
        //wait for xhci controller to set cycle bit to current CCS value
        if (TRB_CTRL_C_BITS(event_trb->control) == ring->cs) {
            break;
        }

        //wait
        wait_ms(250);
        timeout+=250;
        LOG_INFO("current TRB control value: %d\n\r", event_trb->control);
        if (timeout == 1000) {
            LOG_ERROR("timeout on TRB completion\n\r");
            return TRB_INVALID;
            break;
        }
    }
    
    LOG_INFO("TRB completion detected.\n\r");
    LOG_INFO("TRB ptr is 0x%lx\n\r", event_trb->data_ptr);
    LOG_INFO("TRB Status is 0x%x\n\r", event_trb->status);
    LOG_INFO("TRB CONTROL is 0x%x\n\r", event_trb->control);
    
    

    ring->dq_nxt = get_next_trb(ring, ring->dq_nxt, 1);

    write_runtime_reg(ctrl, XHCI_IR_ERDP(0), ((u64)ring->dq_nxt) & 0xFFFFFFFF);
    write_runtime_reg(ctrl, XHCI_IR_ERDP(0)+4, (((u64)ring->dq_nxt)>>32) & 0xFFFFFFFF);
    return TRB_STATUS_CC_GET_BITS(event_trb->status);
}


int execute_xhci_noop(struct usb_ctrl *ctrl) {
    //db stream id field is 0
    //db target field is "host controller command value"
    struct xhci_trb *nxt;
    int ret;
    nxt = ctrl->cmdring->enq_nxt;




    //check if ring is full
    if (get_next_trb(ctrl->cmdring, ctrl->cmdring->enq_nxt, 0) == ctrl->cmdring->dq_nxt) {
        //TODO: make this wait for the ring to be empty.
        LOG_ERROR("TRB ring is full\n\r");
        return TRB_INVALID;
    }
    nxt->data_ptr = 0;
    nxt->status = 0;
    nxt->control = TRB_CTRL_TYPE_BITS(TRB_TYPE_CMD_NOOP);
    asm volatile("dmb sy" ::: "memory");

    LOG_INFO("executing NOOP cmd \n\r");
    //set cycle bit to advance enqueue pointer, and get the next trb in the ring
    nxt->control = set_bits(nxt->control, ctrl->cmdring->cs, TRB_CTRL_C_LSB, 1);
    
    LOG_INFO("next trb address is 0x%lx\n\r", nxt);
    LOG_INFO("TRB ptr is 0x%lx\n\r", nxt->data_ptr);
    LOG_INFO("TRB Status is 0x%x\n\r", nxt->status);
    LOG_INFO("TRB CONTROL is 0x%x\n\r", nxt->control);
    ctrl->cmdring->enq_nxt = get_next_trb(ctrl->cmdring, ctrl->cmdring->enq_nxt, 1);

    write_db_reg(ctrl, 0, DB_TGT_CMD);

    ret =  poll_event_queue(ctrl, ctrl->primary_event_ring);

    LOG_INFO("iman register: 0x%x\n\r", read_runtime_reg(ctrl, XHCI_IMAN(0)));
    LOG_INFO("iman register: 0x%x\n\r", read_runtime_reg(ctrl, XHCI_IR_ERDP(0)));
    return ret;
}


struct device* xhci_probe(void *base_addr) {
    struct usb_ctrl *ctrl;
    struct device *dev;
    u64 reg;
    void *memory = kmalloc(64*1024);
    
    dev = (struct device *)memory;

    dev->device_struct = dev->name + sizeof(dev->name);
    memcpy(dev->name, "xhci_usb_ctrl", sizeof("xhci_usb_ctrl"));

    ctrl = (struct usb_ctrl *)dev->device_struct;
    ctrl->mmio_base = base_addr;
    ctrl->ops = xhci_ops;

    ctrl->operational_reg_base = xhci_read32(ctrl, CAPLENGTH_OFF) & CAPLENGTH_MASK;
    ctrl->doorbell_array_base = xhci_read32(ctrl, DBOFF); 
    ctrl->runtime_reg_base = xhci_read32(ctrl, RTSOFF); //ru
   
    LOG_INFO("operational reg base at %x\n\r",  ctrl->operational_reg_base);
    LOG_INFO("doorbell reg base at %x\n\r",  ctrl->doorbell_array_base);
    LOG_INFO("runtime reg base at %x\n\r",  ctrl->runtime_reg_base);

    if (wait_xhci_rdy(ctrl)) {
        LOG_ERROR("Timed out waiting for xhci ready\n\r");
        return NULL;
    }

    LOG_INFO("halting xhci controller\n\r");
    if (halt_xhci_ctrl(ctrl)) {
        LOG_ERROR("Failed to halt XHCI controller\n\r");
        return NULL;
    }
   
    if (reset_xhci_ctrl(ctrl) != 0) {
        return NULL;
    }

    //get max number of ports, interrupts, and devices
    // Max devices: Physical max number of device slots tht can be in the context structures and doorbell array.
    reg = xhci_read32(ctrl, HCSPARAMS1);
    ctrl->max_ports = get_bits_mask(reg, HCSPARAMS1_MAXPORTS_MASK, HCSPARAMS1_MAXPORTS_LSB);
    ctrl->max_intrs = get_bits_mask(reg, HCSPARAMS1_MAXINTRS_MASK, HCSPARAMS1_MAXINTRS_LSB);
    ctrl->max_devices = get_bits_mask(reg, HCSPARAMS1_MAXSLOTS_MASK, HCSPARAMS1_MAXSLOTS_LSB);

    reg = xhci_read32(ctrl, HCSPARAMS2);
    ctrl->max_erst = get_bits_mask(reg, HCSPARAMS2_ERSTMAX_MASK, HCSPARAMS2_ERSTMAX_LSB);
    ctrl->max_erst = 1 << ctrl->max_erst;
    
    //enable all device slots
    write_opreg(ctrl, CONFIG, ctrl->max_devices);

    if (allocate_device_ctx_arr(ctrl)) {
        LOG_ERROR("Could not allocate device context array\n\r");
        return NULL;
    }
   
    LOG_INFO("Device ctx array at 0x%lx\n\r", ctrl->device_context_arr);
    for (int i = 0; i < ctrl->max_devices; i++) {
        u64 *entry;
        entry = (u64*)(ctrl->device_context_arr + (i*8));
        LOG_INFO("Context device Entry %d: 0x%lx\n\r",  i, *entry);
    }

    ctrl->cmdring = allocate_trb_ring(ctrl, TRB_RING_MIN_SEGMENT_SIZE, XHCI_TRB_RING_ALIGN);
    if (ctrl->cmdring == NULL) {
        free_device_ctx_arr(ctrl);
        return NULL;
    }

    LOG_INFO("cmd ring at 0x%lx, head at 0x%lx\n\r", ctrl->cmdring, ctrl->cmdring->head);

    ctrl->primary_event_ring = allocate_trb_ring(ctrl, TRB_RING_MIN_SEGMENT_SIZE, XHCI_TRB_RING_ALIGN);
     if (ctrl->primary_event_ring == NULL) {
        free_trb_ring(ctrl->cmdring);
        free_device_ctx_arr(ctrl);
        return NULL;
    }
    
    LOG_INFO("primary event ring at 0x%lx, head at 0x%lx\n\r", ctrl->primary_event_ring, ctrl->primary_event_ring->head);

    //command ring is guaranteed to be halted since we cleared r/s bit
    reg = read_opreg(ctrl, CRCR);
    if ((reg & CRCR_CRR_MASK) != 0)  {
        LOG_ERROR("Command Ring running during initialization\n\r");
    }
    //Set cmd ring buffer pointer
    //  initialize RCS to 1 as per XHCI spec 4.9.2.2
    //cycle bit is read-only by
    reg = ((u64)ctrl->cmdring->head & CRCR_CRPTR_MASK) | CRCR_RCS_MASK;
    LOG_INFO("Writing 0x%lx to CRCR\n\r", (u32)reg);
    LOG_INFO("Writing 0x%lx to CRCR+4\n\r",(u32)(reg>>32));
    write_opreg(ctrl, CRCR, (u32)reg);
    write_opreg(ctrl, CRCR+4, (u32)(reg>>32));
    
    write_runtime_reg(ctrl, XHCI_IMAN(0), 2);

    //allocate a 1 segment ERS table of size 16 TRB entries
    ctrl->ers_table =  allocate_ers_table(ctrl, 1);
    ctrl->ers_table->head->ring_segment_base = (u64)ctrl->primary_event_ring->head;
    ctrl->ers_table->head->ring_segment_size = TRB_RING_MIN_SEGMENT_SIZE;
    //for now only support 1 segment on the TRB command ring
    write_runtime_reg(ctrl, XHCI_IR_ERSTSZ(0), 1);
    

    //write the head of the ers_table to the ERSTBA register for the XHCI controller to know where the first
    //event ring segment is. Write ERSTBA last since it initializes the event ring state machine
    reg = (u64)ctrl->primary_event_ring->head;
    write_runtime_reg(ctrl, XHCI_IR_ERDP(0), (u32)(reg & 0xFFFFFFC0));
    write_runtime_reg(ctrl, XHCI_IR_ERDP(0) + 4, (u32)((reg>>32) & 0xFFFFFFFF));
    reg =  (u64)(ctrl->ers_table->head);
    write_runtime_reg(ctrl, XHCI_IR_ERSTBA(0), (u32)(reg & 0xFFFFFFC0));
    write_runtime_reg(ctrl, XHCI_IR_ERSTBA(0) + 4, (u32)((reg>>32) & 0xFFFFFFFF));

   

    //make sure all previous writes finish before enabling the controller
    asm volatile("dmb sy" ::: "memory");

    LOG_INFO("starting xhci controller\n\r");
    if (enable_xhci_ctrl(ctrl) != 0 ) {
        return NULL;
    }

    
    LOG_INFO("%d ports, %d intrs, %d devices\n\r", ctrl->max_ports, ctrl->max_intrs, ctrl->max_devices);
    for (int i = 0; i < ctrl->max_ports; i++) {
        LOG_INFO("PORT %d Status: 0x%lx\n\r", i, read_opreg(ctrl, 0x400 + (0x10*i)));
    }
    

    dump_ctrl_regs(ctrl);

    reg = execute_xhci_noop(ctrl);

    LOG_INFO("Completino code: %lx\n\r", reg);

    dump_ctrl_regs(ctrl);
    dump_all_tables(ctrl);
    return dev;
}