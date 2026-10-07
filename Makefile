CC := clang
LD := lld-link

CCFLAGS := -Iinclude/efi -Iinclude/kboot -target x86_64-pc-win32-coff -fno-stack-protector -fshort-wchar -mno-red-zone -masm=att -Wno-incompatible-library-redeclaration -Wno-shift-count-overflow
LDFLAGS := -subsystem:efi_application -nodefaultlib -dll

SRC := src
BUILD := build
OUT := build/out

BIN := $(OUT)/EFI/BOOT/BOOTX64.EFI
VARS := OVMF_VARS.4m.fd
CFG := $(OUT)/KBOOT/KBOOT.CFG

ALL_C := $(shell find $(SRC) -type f -name "*.c")
ALL_OBJ := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(ALL_C))

.PHONY: all clean run debug

all: $(BIN)

debug: $(BIN) $(VARS) $(CFG)
	qemu-system-x86_64 \
		-enable-kvm \
		-cpu host \
		-m 4G \
		-s -S \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/x64/OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,file=./$(VARS) \
		-drive file=fat:rw:$(OUT),format=raw,id=bootdrive,if=none \
		-device virtio-blk-pci,drive=bootdrive,bootindex=1 &
		gdb --silent -ex "target remote localhost:1234"

run: $(BIN) $(VARS) $(CFG)
	qemu-system-x86_64 \
		-enable-kvm \
		-cpu host \
		-m 4G \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/x64/OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,file=./$(VARS) \
		-drive file=fat:rw:$(OUT),format=raw,id=bootdrive,if=none \
		-device virtio-blk-pci,drive=bootdrive,bootindex=1

clean:
	rm -rf $(BUILD)


$(BIN): $(ALL_OBJ)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -entry:efi_main $^ -out:$@
	@printf "LD        $(notdir $@)\n"

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS) -c $< -o $@
	@printf "CC        $(notdir $<)\n"

$(VARS):
	@cp /usr/share/ovmf/x64/$@ .
	@printf "CP        OVMF_VARS\n"

$(CFG):
	@mkdir -p $(dir $@)
	@cp resources/KBOOT.CFG $@
	@printf "CP        resources/KBOOT.CFG -> $@\n"

