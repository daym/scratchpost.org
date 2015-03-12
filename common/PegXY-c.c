/* Contains evil evil make-it-faster hacks. Not endian-safe. */
#include <stdint.h>

#pragma pack(1)

struct NoteHeader {
	uint16_t next_pointer_L;
	uint8_t next_pointer_H;
	uint8_t flags;
	uint8_t note_number;
	uint8_t total_note_number;
	uint8_t __reserved[8];
};

struct XY {
	int16_t X;
	int16_t Y;
};

static int note_XY_present_P(uint8_t flags) {
	return !(flags & 0x80);
}

static int note_open_P(uint8_t flags) {
	return flags & 0x40;
}

static int note_software_open_P(uint8_t flags) {
	return !($flags & 0x20);
}

static int header_last_P(const struct NoteHeader* header) {
	return (header->next_pointer_L == 0xFFFF && header->next_pointer_H == 0xFF) || header->next_pointer == 0;
}

static int XY_pen_up_P(const struct XY* XY) {
	return (XY->Y == -32768 && XY->X == 0);
}

void transfer_page(FILE* input_file, struct PageHeader* header, FILE* output_file) {
	unsigned position = ftell(input_file);
	unsigned next_pointer;
	struct XY XY;
	int status;
	if(header)
		next_pointer = header->next_pointer_L + 65536 * header->next_pointer_H;
	else
		next_pointer = 0;
	if(!note_XY_present_P(header->flags))
		return;

	assert(next_pointer >= position);
	while(ftell(input_file) < next_pointer) {
		status = fread(&XY, sizeof(XY), 1, input_file);
		assert(status == 1);
		if(XY_pen_up_P(&XY)) {
		} else {
		}
	}
}

struct PageHeader next_page(FILE* input_file, const struct PageHeader* header, FILE* output_file) {
	unsigned next_pointer;
	struct PageHeader result;
	int status;
	if(header)
		next_pointer = header->next_pointer_L + 65536 * header->next_pointer_H;
	else
		next_pointer = 0;

	status = fseek(input_file, next_pointer, SEEK_SET);
	assert(status != -1);
	status = fread(&result, sizeof(result), 1, input_file);
	assert(status == 1);
	return result;
}

int main() {
	FILE* input_file;
	struct PageHeader header;

	printf("Content-Type: image/svg+xml\015\012");
	printf("<?xml version=\"1.0\"?>");

	input_file = stdin;
	header = next_page(input_file, NULL, stdout);
	transfer_page(input_file, header, output_file);

	return 0;
}
