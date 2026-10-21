/* ELF64 relocatable writer for the Linux C1 port.
   It consumes the same compiler image buffer as the upstream C1O1 writer:
   generated code starts at image offset 4096, globals live at the nominal
   base 0x600000, and all back-patches become R_X86_64_PC32 relocations. */

int object_symbol_map[2048];
int elf_sym_id[2053];
int elf_sym_name[2053];
int elf_sym_count;
int elf_first_global;
int elf_init_index;

int elf_align(int value, int alignment) {
    return (value + alignment - 1) & -alignment;
}

int elf_put32(int at, unsigned int value) {
    return write32(at, (int)value);
}

int elf_put64(int at, unsigned long long value) {
    write32(at, (int)value);
    return write32(at + 4, (int)(value >> 32));
}

int elf_put_name(int at, int id) {
    int i, n = lengths[id];
    for (i = 0; i < n; i++)
        image[at + i] = names[id * 256 + i];
    image[at + n] = 0;
    return n + 1;
}

int elf_put_text(int at, char *text) {
    int i = 0;
    while (text[i]) {
        image[at + i] = text[i];
        i++;
    }
    image[at + i] = 0;
    return i + 1;
}

int elf_symbol_local(int id) {
    return (symbol_flags[id] & 32) != 0;
}

int elf_symbol_included(int id) {
    return (symbol_flags[id] & 8) || file_function_type[id] || (symbol_flags[id] & 1);
}

int elf_symbol_defined(int id) {
    if (symbol_flags[id] & 8)
        return global_address[id] != 0;
    if (symbol_flags[id] & 1)
        return 1;
    return 0;
}

int elf_symbol_section(int id) {
    if (!elf_symbol_defined(id))
        return 0;
    if (symbol_flags[id] & 1)
        return 1;
    return 3;
}

int elf_symbol_type(int id) {
    if ((symbol_flags[id] & 1) || (file_function_type[id] && !(symbol_flags[id] & 8)))
        return 2;
    return 1;
}

int elf_symbol_value(int id) {
    if (symbol_flags[id] & 1)
        return function_address[id] - 4096;
    if (global_address[id])
        return global_address[id] - 0x600000;
    return 0;
}

int elf_symbol_size(int id) {
    if ((symbol_flags[id] & 8) && global_address[id])
        return global_extent[id];
    return 0;
}

int elf_symbol_binding(int id) {
    if (elf_symbol_local(id))
        return 0;
    return 1;
}

int elf_section_header(int at, int name, int type, unsigned long long flags,
                       unsigned long long addr, unsigned long long offset,
                       unsigned long long size, int link, int info,
                       unsigned long long alignment, unsigned long long entsize) {
    elf_put32(at, name);
    elf_put32(at + 4, type);
    elf_put64(at + 8, flags);
    elf_put64(at + 16, addr);
    elf_put64(at + 24, offset);
    elf_put64(at + 32, size);
    elf_put32(at + 40, link);
    elf_put32(at + 44, info);
    elf_put64(at + 48, alignment);
    elf_put64(at + 56, entsize);
    return 0;
}

int elf_relocation(int at, int offset, int symbol, int addend) {
    elf_put64(at, offset);
    elf_put32(at + 8, 2);
    elf_put32(at + 12, symbol);
    elf_put32(at + 16, addend);
    return elf_put32(at + 20, addend < 0 ? -1 : 0);
}

int object_prepare(void) {
    int i, n, refs, code_size, text_off, rela_off, bss_off, sym_off,
        str_off, shstr_off, shoff, total, str_size, text_size, rela_size,
        sym_size, shstr_size, at, id, section, value, binding, size, type;
    if (initializer_tail) {
        patch(initializer_tail, position);
        emit(0xc3);
    }
    if (position == 4096)
        emit(0xc3);
    code_size = position - 4096;
    refs = call_count + object_ref_count + address_ref_count;
    if (refs > 16384)
        return error_message("ELF relocation capacity exceeded");
    for (i = 0; i < count; i++)
        object_symbol_map[i] = -1;
    n = 3;
    for (i = 2; i < count; i++)
        if (elf_symbol_included(i) && elf_symbol_local(i)) {
            object_symbol_map[i] = n;
            elf_sym_id[n++] = i;
        }
    elf_first_global = n;
    for (i = 2; i < count; i++)
        if (elf_symbol_included(i) && !elf_symbol_local(i)) {
            object_symbol_map[i] = n;
            elf_sym_id[n++] = i;
        }
    elf_init_index = -1;
    if (initializer_start) {
        elf_init_index = n;
        elf_sym_id[n++] = -1;
    }
    elf_sym_count = n;
    for (i = 0; i < call_count; i++) {
        id = call_symbol[i];
        if (object_symbol_map[id] < 0)
            return error_message("ELF call has no symbol");
        if (call_arity[i] >= 0 && !arity_matches(arity[id], call_arity[i]))
            return error();
        if (call_position[i] < 4096 || call_position[i] > position - 4)
            return error();
    }
    for (i = 0; i < object_ref_count; i++)
        if (object_symbol_map[object_ref_symbol[i]] < 0 || object_ref_position[i] < 4096 ||
            object_ref_position[i] > position - 4)
            return error();
    for (i = 0; i < address_ref_count; i++)
        if (address_ref_position[i] < 4096 || address_ref_position[i] > position - 4 ||
            address_ref_target[i] < 0x401e00)
            return error_message("ELF address reference is out of range");
    text_off = 4096;
    text_size = code_size;
    rela_size = refs * 24;
    sym_size = elf_sym_count * 24;
    str_size = 1;
    for (i = 3; i < elf_sym_count; i++) {
        id = elf_sym_id[i];
        if (i == elf_init_index)
            str_size += 10;
        else if (id >= 0)
            str_size += lengths[id] + 1;
    }
    shstr_size = 65;
    rela_off = elf_align(text_off + text_size, 8);
    bss_off = elf_align(rela_off + rela_size, 16);
    sym_off = elf_align(bss_off, 8);
    str_off = sym_off + sym_size;
    shstr_off = str_off + str_size;
    shoff = elf_align(shstr_off + shstr_size, 8);
    total = shoff + 8 * 64;
    if (total > 1049088)
        return error_message("ELF output capacity exceeded");
    for (i = 0; i < text_off; i++)
        image[i] = 0;
    for (i = text_off + text_size; i < total; i++)
        image[i] = 0;
    image[0] = 0x7f;
    image[1] = 'E';
    image[2] = 'L';
    image[3] = 'F';
    image[4] = 2;
    image[5] = 1;
    image[6] = 1;
    elf_put32(16, 1);
    elf_put32(18, 62);
    elf_put32(20, 1);
    elf_put64(24, 0);
    elf_put64(32, 0);
    elf_put64(40, shoff);
    elf_put32(48, 0);
    elf_put32(52, 64);
    elf_put32(54, 0);
    elf_put32(56, 0);
    elf_put32(58, 64);
    elf_put32(60, 8);
    elf_put32(62, 6);
    at = rela_off;
    for (i = 0; i < call_count; i++) {
        elf_relocation(at, call_position[i] - 4096, object_symbol_map[call_symbol[i]], -4);
        at += 24;
    }
    for (i = 0; i < object_ref_count; i++) {
        elf_relocation(at, object_ref_position[i] - 4096,
                       object_symbol_map[object_ref_symbol[i]], -4);
        at += 24;
    }
    for (i = 0; i < address_ref_count; i++) {
        section = address_ref_target[i] >= 0x600000 ? 3 : 1;
        value = address_ref_target[i] - (section == 3 ? 0x600000 : 0x401e00) - 4;
        elf_relocation(at, address_ref_position[i] - 4096, section == 3 ? 2 : 1, value);
        at += 24;
    }
    for (i = 0; i < elf_sym_count * 24; i++)
        image[sym_off + i] = 0;
    for (i = 3; i < elf_sym_count; i++) {
        id = elf_sym_id[i];
        at = sym_off + i * 24;
        if (i == elf_init_index) {
            image[at + 4] = 0x12;
            image[at + 5] = 0;
            image[at + 6] = 1;
            image[at + 7] = 0;
            elf_put32(at + 8, initializer_start - 4096);
            elf_put32(at + 12, 0);
            elf_put32(at + 16, 0);
            elf_put32(at + 20, 0);
            continue;
        }
        section = elf_symbol_section(id);
        value = elf_symbol_value(id);
        size = elf_symbol_size(id);
        binding = elf_symbol_binding(id);
        type = elf_symbol_type(id);
        image[at + 4] = (binding << 4) | type;
        image[at + 5] = 0;
        image[at + 6] = section & 255;
        image[at + 7] = section >> 8;
        elf_put32(at + 8, value);
        elf_put32(at + 12, value < 0 ? -1 : 0);
        elf_put32(at + 16, size);
        elf_put32(at + 20, 0);
    }
    image[sym_off + 24 + 4] = 3;
    image[sym_off + 24 + 6] = 1;
    image[sym_off + 48 + 4] = 3;
    image[sym_off + 48 + 6] = 3;
    at = str_off + 1;
    for (i = 3; i < elf_sym_count; i++) {
        id = elf_sym_id[i];
        if (i == elf_init_index) {
            elf_sym_name[i] = at - str_off;
            image[at++] = '_';
            image[at++] = '_';
            image[at++] = 'c';
            image[at++] = '1';
            image[at++] = '_';
            image[at++] = 'i';
            image[at++] = 'n';
            image[at++] = 'i';
            image[at++] = 't';
            image[at++] = 0;
        } else {
            elf_sym_name[i] = at - str_off;
            at += elf_put_name(at, id);
        }
    }
    for (i = 3; i < elf_sym_count; i++)
        elf_put32(sym_off + i * 24, elf_sym_name[i]);
    image[shstr_off] = 0;
    at = shstr_off + 1;
    at += elf_put_text(at, ".text");
    at += elf_put_text(at, ".rela.text");
    at += elf_put_text(at, ".bss");
    at += elf_put_text(at, ".symtab");
    at += elf_put_text(at, ".strtab");
    at += elf_put_text(at, ".shstrtab");
    elf_put_text(at, ".note.GNU-stack");
    elf_section_header(shoff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    elf_section_header(shoff + 64, 1, 1, 6, 0, text_off, text_size, 0, 0, 16, 0);
    elf_section_header(shoff + 128, 7, 4, 0, 0, rela_off, rela_size, 4, 1, 8, 24);
    elf_section_header(shoff + 192, 18, 8, 3, 0, bss_off,
                       allocation > 0x600000 ? allocation - 0x600000 : 1, 0, 0, 16, 0);
    elf_section_header(shoff + 256, 23, 2, 0, 0, sym_off, sym_size, 5, elf_first_global, 8, 24);
    elf_section_header(shoff + 320, 31, 3, 0, 0, str_off, str_size, 0, 0, 1, 0);
    elf_section_header(shoff + 384, 39, 3, 0, 0, shstr_off, shstr_size, 0, 0, 1, 0);
    elf_section_header(shoff + 448, 49, 1, 0, 0, 0, 0, 0, 0, 1, 0);
    return total;
}

int configure_object(void) {
    object_mode = 1;
    return 0;
}
