package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"os"
	"slices"
)

// BinaryReader is a helper for reading binary data with the correct endianness
type BinaryReader struct {
	reader io.Reader
	order  binary.ByteOrder
}

// NewBinaryReader creates a new BinaryReader
func NewBinaryReader(r io.Reader, order binary.ByteOrder) *BinaryReader {
	return &BinaryReader{reader: r, order: order}
}

// Read reads data into v
func (br *BinaryReader) Read(data interface{}) error {
	return binary.Read(br.reader, br.order, data)
}

// ELFSectionHeader represents a single ELF section header
type ELFSectionHeader struct {
	sh_name      uint32
	sh_type      uint32
	sh_flags     uint64
	sh_addr      uint64
	sh_offset    uint64
	sh_size      uint64
	sh_link      uint32
	sh_info      uint32
	sh_addralign uint64
	sh_entsize   uint64
	name         string
}

// ELFFile represents the parsed ELF file structure
type ELFFile struct {
	is64Bit        bool
	byteOrder      binary.ByteOrder
	sectionHeaders []*ELFSectionHeader
	shstrndx       uint16
}

// DWARFSections contains all DWARF debug sections
type DWARFSections struct {
	abbrev      []byte
	line        []byte
	info        []byte
	addr        []byte
	aranges     []byte
	frame       []byte
	eh_frame    []byte
	line_str    []byte
	loc         []byte
	loclists    []byte
	names       []byte
	macinfo     []byte
	macro       []byte
	pubnames    []byte
	pubtypes    []byte
	ranges      []byte
	rnglists    []byte
	str         []byte
	str_offsets []byte
	types       []byte
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: go run elf_parser.go <elf_file>")
		os.Exit(1)
	}

	filePath := os.Args[1]

	// Parse the ELF file
	elf, sections, err := parseELF(filePath)
	if err != nil {
		fmt.Printf("Error parsing ELF file: %v\n", err)
		os.Exit(1)
	}

	// Print summary
	fmt.Printf("Successfully parsed ELF file: %s\n", filePath)
	fmt.Printf("64-bit: %v\n", elf.is64Bit)
	fmt.Printf("Section count: %d\n", len(elf.sectionHeaders))

	// Print DWARF sections found
	printDWARFSections(sections)

	// Check if it's a PIE executable
	isPIE := checkPIE(elf, filePath)
	fmt.Printf("Position Independent Executable (PIE): %v\n", isPIE)
}

// parseELF parses an ELF file and extracts DWARF sections
func parseELF(filePath string) (*ELFFile, *DWARFSections, error) {
	// Read the entire file
	fileBuf, err := os.ReadFile(filePath)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to read file: %w", err)
	}

	reader := NewBinaryReader(bytes.NewBuffer(fileBuf), binary.LittleEndian)

	// Check ELF magic number
	magic := []byte{0x7f, 'E', 'L', 'F'}
	magicBuf := make([]byte, len(magic))
	reader.readBytes(magicBuf)

	if !slices.Equal(magic, magicBuf) {
		return nil, nil, fmt.Errorf("incorrect ELF magic number")
	}

	// Parse e_ident
	var class, data, version, osabi, abiversion uint8
	reader.Read(&class)
	reader.Read(&data)
	reader.Read(&version)
	reader.Read(&osabi)
	reader.Read(&abiversion)

	// Skip padding (7 bytes)
	reader.readBytes(make([]byte, 7))

	// Determine if 64-bit and byte order
	is64Bit := class == 2 // 1=32-bit, 2=64-bit

	var byteOrder binary.ByteOrder
	if data == 1 { // 1=little endian, 2=big endian
		byteOrder = binary.LittleEndian
	} else {
		byteOrder = binary.BigEndian
	}

	// Update reader with correct byte order
	reader.order = byteOrder

	// Read rest of ELF header
	var eType, eMachine uint16
	var eVersion uint32
	var eEntry, ePhoff, eShoff uint64
	var eFlags uint32
	var eEhsize, ePhentsize, ePhnum, eShentsize, eShnum, eShstrndx uint16

	if is64Bit {
		reader.Read(&eType)
		reader.Read(&eMachine)
		reader.Read(&eVersion)
		reader.Read(&eEntry)
		reader.Read(&ePhoff)
		reader.Read(&eShoff)
		reader.Read(&eFlags)
		reader.Read(&eEhsize)
		reader.Read(&ePhentsize)
		reader.Read(&ePhnum)
		reader.Read(&eShentsize)
		reader.Read(&eShnum)
		reader.Read(&eShstrndx)
	} else {
		// For 32-bit, fields are 32-bit
		var eEntry32, ePhoff32, eShoff32 uint32
		reader.Read(&eType)
		reader.Read(&eMachine)
		reader.Read(&eVersion)
		reader.Read(&eEntry32)
		reader.Read(&ePhoff32)
		reader.Read(&eShoff32)
		reader.Read(&eFlags)
		reader.Read(&eEhsize)
		reader.Read(&ePhentsize)
		reader.Read(&ePhnum)
		reader.Read(&eShentsize)
		reader.Read(&eShnum)
		reader.Read(&eShstrndx)

		eEntry = uint64(eEntry32)
		ePhoff = uint64(ePhoff32)
		eShoff = uint64(eShoff32)
	}

	// Parse section headers
	sectionHeaders, err := parseSectionHeaders(fileBuf, eShoff, eShnum, eShentsize, is64Bit, byteOrder)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to parse section headers: %w", err)
	}

	// Get section names
	err = parseSectionNames(fileBuf, sectionHeaders, eShstrndx, byteOrder)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to parse section names: %w", err)
	}

	// Extract DWARF sections
	sections := extractDWARFSections(fileBuf, sectionHeaders)

	elf := &ELFFile{
		is64Bit:        is64Bit,
		byteOrder:      byteOrder,
		sectionHeaders: sectionHeaders,
		shstrndx:       eShstrndx,
	}

	return elf, sections, nil
}

// parseSectionHeaders parses the ELF section header table
func parseSectionHeaders(fileBuf []byte, shoff uint64, shnum, shentsize uint16, is64Bit bool, order binary.ByteOrder) ([]*ELFSectionHeader, error) {
	start := int(shoff)
	end := start + int(shentsize)*int(shnum)

	if end > len(fileBuf) {
		return nil, fmt.Errorf("section header table exceeds file size")
	}

	sectionTable := bytes.NewBuffer(fileBuf[start:end])
	reader := NewBinaryReader(sectionTable, order)

	headers := make([]*ELFSectionHeader, 0, shnum)

	for i := 0; i < int(shnum); i++ {
		header := &ELFSectionHeader{}

		if is64Bit {
			reader.Read(&header.sh_name)
			reader.Read(&header.sh_type)
			reader.Read(&header.sh_flags)
			reader.Read(&header.sh_addr)
			reader.Read(&header.sh_offset)
			reader.Read(&header.sh_size)
			reader.Read(&header.sh_link)
			reader.Read(&header.sh_info)
			reader.Read(&header.sh_addralign)
			reader.Read(&header.sh_entsize)
		} else {
			// 32-bit fields
			var sh_name, sh_type uint32
			var sh_flags, sh_addr, sh_offset, sh_size uint32
			var sh_link, sh_info uint32
			var sh_addralign, sh_entsize uint32

			reader.Read(&sh_name)
			reader.Read(&sh_type)
			reader.Read(&sh_flags)
			reader.Read(&sh_addr)
			reader.Read(&sh_offset)
			reader.Read(&sh_size)
			reader.Read(&sh_link)
			reader.Read(&sh_info)
			reader.Read(&sh_addralign)
			reader.Read(&sh_entsize)

			header.sh_name = sh_name
			header.sh_type = sh_type
			header.sh_flags = uint64(sh_flags)
			header.sh_addr = uint64(sh_addr)
			header.sh_offset = uint64(sh_offset)
			header.sh_size = uint64(sh_size)
			header.sh_link = sh_link
			header.sh_info = sh_info
			header.sh_addralign = uint64(sh_addralign)
			header.sh_entsize = uint64(sh_entsize)
		}

		headers = append(headers, header)
	}

	return headers, nil
}

// parseSectionNames extracts section names from the string table
func parseSectionNames(fileBuf []byte, headers []*ELFSectionHeader, shstrndx uint16, order binary.ByteOrder) error {
	if int(shstrndx) >= len(headers) {
		return fmt.Errorf("invalid string table index")
	}

	strHeader := headers[shstrndx]
	start := int(strHeader.sh_offset)
	end := start + int(strHeader.sh_size)

	if end > len(fileBuf) {
		return fmt.Errorf("string table exceeds file size")
	}

	sectionNamesBuf := fileBuf[start:end]

	for _, header := range headers {
		nameOffset := int(header.sh_name)
		if nameOffset < len(sectionNamesBuf) {
			// Find null-terminated string
			for i := nameOffset; i < len(sectionNamesBuf); i++ {
				if sectionNamesBuf[i] == 0 {
					header.name = string(sectionNamesBuf[nameOffset:i])
					break
				}
			}
		}
	}

	return nil
}

// extractDWARFSections extracts all DWARF debug sections
func extractDWARFSections(fileBuf []byte, headers []*ELFSectionHeader) *DWARFSections {
	sections := &DWARFSections{}

	for _, header := range headers {
		start := int(header.sh_offset)
		end := start + int(header.sh_size)

		if start >= len(fileBuf) || end > len(fileBuf) {
			continue
		}

		sectionData := fileBuf[start:end]

		switch header.name {
		case ".debug_abbrev":
			sections.abbrev = sectionData
		case ".debug_line":
			sections.line = sectionData
		case ".debug_info":
			sections.info = sectionData
		case ".debug_addr":
			sections.addr = sectionData
		case ".debug_aranges":
			sections.aranges = sectionData
		case ".debug_frame":
			sections.frame = sectionData
		case ".eh_frame":
			sections.eh_frame = sectionData
		case ".debug_line_str":
			sections.line_str = sectionData
		case ".debug_loc":
			sections.loc = sectionData
		case ".debug_loclists":
			sections.loclists = sectionData
		case ".debug_names":
			sections.names = sectionData
		case ".debug_macinfo":
			sections.macinfo = sectionData
		case ".debug_macro":
			sections.macro = sectionData
		case ".debug_pubnames":
			sections.pubnames = sectionData
		case ".debug_pubtypes":
			sections.pubtypes = sectionData
		case ".debug_ranges":
			sections.ranges = sectionData
		case ".debug_rnglists":
			sections.rnglists = sectionData
		case ".debug_str":
			sections.str = sectionData
		case ".debug_str_offsets":
			sections.str_offsets = sectionData
		case ".debug_types":
			sections.types = sectionData
		}
	}

	return sections
}

// checkPIE checks if the executable is position independent
func checkPIE(elf *ELFFile, fileBuf []byte) bool {
	// Find .dynamic section
	var dynamicHeader *ELFSectionHeader
	for _, header := range elf.sectionHeaders {
		if header.name == ".dynamic" {
			dynamicHeader = header
			break
		}
	}

	if dynamicHeader == nil {
		return false
	}

	start := int(dynamicHeader.sh_offset)
	end := start + int(dynamicHeader.sh_size)

	if start >= len(fileBuf) || end > len(fileBuf) {
		return false
	}

	dynamicBuf := bytes.NewBuffer(fileBuf[start:end])
	reader := NewBinaryReader(dynamicBuf, elf.byteOrder)

	for {
		var tag, val uint64

		if elf.is64Bit {
			if err := reader.Read(&tag); err != nil {
				break
			}
			if err := reader.Read(&val); err != nil {
				break
			}
		} else {
			var tag32, val32 uint32
			if err := reader.Read(&tag32); err != nil {
				break
			}
			if err := reader.Read(&val32); err != nil {
				break
			}
			tag = uint64(tag32)
			val = uint64(val32)
		}

		// DT_FLAGS_1 = 0x6ffffffb
		if tag == 0x6ffffffb {
			// DF_1_PIE = 0x08000000
			if (val & 0x08000000) > 0 {
				return true
			}
		}

		// DT_NULL marks end of dynamic section
		if tag == 0 {
			break
		}
	}

	return false
}

// printDWARFSections prints a summary of found DWARF sections
func printDWARFSections(sections *DWARFSections) {
	fmt.Println("\nDWARF sections found:")

	sectionsMap := map[string][]byte{
		".debug_abbrev":      sections.abbrev,
		".debug_line":        sections.line,
		".debug_info":        sections.info,
		".debug_addr":        sections.addr,
		".debug_aranges":     sections.aranges,
		".debug_frame":       sections.frame,
		".eh_frame":          sections.eh_frame,
		".debug_line_str":    sections.line_str,
		".debug_loc":         sections.loc,
		".debug_loclists":    sections.loclists,
		".debug_names":       sections.names,
		".debug_macinfo":     sections.macinfo,
		".debug_macro":       sections.macro,
		".debug_pubnames":    sections.pubnames,
		".debug_pubtypes":    sections.pubtypes,
		".debug_ranges":      sections.ranges,
		".debug_rnglists":    sections.rnglists,
		".debug_str":         sections.str,
		".debug_str_offsets": sections.str_offsets,
		".debug_types":       sections.types,
	}

	for name, data := range sectionsMap {
		if len(data) > 0 {
			fmt.Printf("  %-20s: %8d bytes\n", name, len(data))
		}
	}
}

// Helper method for BinaryReader to read bytes
func (br *BinaryReader) readBytes(buf []byte) error {
	_, err := io.ReadFull(br.reader, buf)
	return err
}
