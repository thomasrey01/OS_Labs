#pragma once

// More user-friendly version of the elf.h header that ships with Linux natively

#include <stdint.h>

typedef uint8_t byte;

// First we define some constants that are relevant for us
#define ELF_MAGIC 0x464C457FU // "\x7FELF" in little endian
#define ELF_VERSION 1
#define ELF_MACHINE_x64 0x3E // we only allow x64 target

// The enums relevant for our headers

// ELF main header type
enum elf_hdr_type {
    ELF_TYPE_REL = 1, // relocatable file
    ELF_TYPE_EXEC = 2, // executable file
    ELF_TYPE_DYN = 3, // dynamic library (but also actually an executable)
    ELF_TYPE_CORE = 4
};

// ELF program header type
enum elf_proghdr_type {
    ELF_PROG_NULL = 0,
    ELF_PROG_LOAD = 1, // this section should be loaded
    ELF_PROG_DYNAMIC = 2, // this tells us about the dynamic linking that needs to be done
    ELF_PROG_INTERP = 3 // this tells us which interpreter to use
};

// ELF program header permission flags
enum elf_proghdr_flag {
    ELF_PROG_FLAG_EXEC = 0b001,
    ELF_PROG_FLAG_WRITE = 0b010,
    ELF_PROG_FLAG_READ = 0b100,
};

// ELF section header types
enum elf_secthdr_type {
    ELF_SECT_NULL = 0,
    ELF_SECT_PROGBITS = 1,
    ELF_SECT_SYMTAB = 2,
    ELF_SECT_STRTAB = 3
};

// ELF auxiliary information vector type
enum elf_auxvec_type {
    AT_NULL = 0,      // Always the last element, indicating end of vector
    AT_IGNORE = 1,    // Should be ignored
    AT_EXECFD = 2,    // File descriptor of the program
    AT_PHDR = 3,      // Pointer to program headers of the program
    AT_PHENT = 4,     // Size of each program header entry
    AT_PHNUM = 5,     // Number of program headers
    AT_PAGESZ = 6,    // System page size
    AT_BASE = 7,      // First address of memory where interpreter is loaded
    AT_FLAGS = 8,     // Some flags
    AT_ENTRY = 9,     // Pointer to entrypoint of the program
    AT_NOTELF = 10,   // When the program is NOT an ELF binary
    AT_UID = 11,      // UID
    AT_EUID = 12,     // Effective UID
    AT_GID = 13,      // GID
    AT_EGID = 14,     // Effective GID
    AT_CLKTCK = 17,   // Frequency at which times() increments
    AT_PLATFORM = 15, // String identifying the CPU we're running
    AT_EXECFN = 31,   // Filename of the progam
    AT_SYSINFO = 32   // Pointer to a global system page for system calls
};

/* The structures actually used in the ELF headers */

// The main ELF header, located at the start of each ELF binary
struct elf_header {
    uint32_t magic;             // Value that indicates this is an ELF binary, must always match ELF_MAGIC
    byte __padding[12];         // Padding to align with a 16-byte boundary
    enum elf_hdr_type type : 16;// The binary type - set to be 16 bits (2 bytes) long
    uint16_t machine;           // We will only accept ELF_MACHINE_x64
    uint32_t version;           // We will only accept ELF_VERSION (the only version now)
    uint64_t entry;             // Pointer to entrypoint of the program in "filespace"
    uint64_t proghdr_offset;    // Pointer to the program headers of this binary in "filespace"
    uint64_t secthdr_offset;    // Pointer to the section headers of this binary in "filespace"
    uint32_t flags;             // Some flags
    uint16_t header_size;       // The size of this header
    uint16_t proghdr_size;      // The size of each program header
    uint16_t proghdr_count;     // The number of program headers present
    uint16_t secthdr_size;      // The size of each section header
    uint16_t secthdr_count;     // The number of section headers present
    uint16_t sectname_mapping;  // The index of the section where the mapping between section index and name is located
} __attribute__((packed));

// The structure of the program header, located at proghdr_offset
struct elf_proghdr {
    enum elf_proghdr_type type : 32;    // The type of this program header - must be 4 bytes long
    enum elf_proghdr_flag flags : 32;   // Some flags for this program header - again 4 bytes
    uint64_t file_offset;               // Pointer to the contents of this segment in "filespace"
    uint64_t virtual_address;           // The preferred virtual memory address to load this segment - an absolute value for non-relocatable code, but relative for relocatable code
    uint64_t physical_address;          // For machines without page tables, the preferred physical memory address to load this segment
    uint64_t size_file;                 // The size of this segment in "filespace"
    uint64_t size_memory;               // The desired size of this segment in memory (might not equal size_file!)
    uint64_t alignment;                 // The desired alignment of this segment
} __attribute__((packed));

// The structure of the section header, located at secthdr_offset
struct elf_secthdr {
    uint32_t name;                      // Pointer to a string with the name of this section in "filespace"
    enum elf_secthdr_type type : 32;    // The type of this section header - 4 bytes
    uint64_t flags;                     // Some flags
    uint64_t virtual_address;           // The address this section is assmed to be at
    uint64_t file_offset;               // Pointer to the contents of this section in "filespace"
    uint64_t size;                      // The size of this section on disk
    uint32_t link;                      // Used for linking sections together
    uint32_t info;                      // Used for storing additional information
    uint64_t alignment;                 // The desired alignment of this section
    uint64_t entry_size;                // If a section describes a list of "entries", the size of each entry
} __attribute__((packed));

// The structure for the auxiliary elf vectors, located on the stack right after the environment values
struct elf_auxvec {
    uint64_t type;  // The type of this vector (ref. the enum above)
    uint64_t value; // The value of this item
};
