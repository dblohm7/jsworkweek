#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define CREATE_FIELD_INFO(obj, field) field_info(#obj "." #field, sizeof(obj.field), reinterpret_cast<char*>(&obj.field) - reinterpret_cast<char*>(&obj))

struct field_info
{
    field_info(const char *name, unsigned int size, unsigned int offset)
	:name(name),
	 size(size),
	 offset(offset)
    {
    }
    string name;
    unsigned int size;
    unsigned int offset;
    bool operator<(const field_info &rhs) const
    {
	return this->offset < rhs.offset;
    }
};

#define PRINT_FIELD_SIZE(obj, field) cout << #obj << "." << #field << ": Size is " << sizeof(obj.field) << ", offset is " << (reinterpret_cast<char*>(&obj.field) - reinterpret_cast<char*>(&obj)) << endl

void check_for_holes(const unsigned int offset, unsigned int &expected_offset)
{
    if (offset > expected_offset) {
	unsigned int hole_size = offset - expected_offset;
	cout << hole_size << " byte hole at offset " << expected_offset << endl;
	expected_offset += hole_size;
    }
}

int main(int argc, char *argv[])
{
    struct stat st;
    vector<field_info> vec;

    vec.push_back(CREATE_FIELD_INFO(st, st_dev));
    vec.push_back(CREATE_FIELD_INFO(st, st_ino));
    vec.push_back(CREATE_FIELD_INFO(st, st_mode));
    vec.push_back(CREATE_FIELD_INFO(st, st_nlink));
    vec.push_back(CREATE_FIELD_INFO(st, st_uid));
    vec.push_back(CREATE_FIELD_INFO(st, st_gid));
    vec.push_back(CREATE_FIELD_INFO(st, st_rdev));
    vec.push_back(CREATE_FIELD_INFO(st, st_size));
    vec.push_back(CREATE_FIELD_INFO(st, st_atime));
    vec.push_back(CREATE_FIELD_INFO(st, st_mtime));
    vec.push_back(CREATE_FIELD_INFO(st, st_ctime));
    vec.push_back(CREATE_FIELD_INFO(st, st_blksize));
    vec.push_back(CREATE_FIELD_INFO(st, st_blocks));

    sort(vec.begin(), vec.end());

    unsigned int expected_offset = 0;
    for (vector<field_info>::const_iterator i = vec.begin(); i != vec.end(); ++i) {
	check_for_holes(i->offset, expected_offset);
	cout << i->name << ": Size is " << i->size << ", offset is " << i->offset << endl;
	expected_offset += i->size;
    }
    check_for_holes(sizeof(st), expected_offset);

    /*
    PRINT_FIELD_SIZE(st, st_dev);
    PRINT_FIELD_SIZE(st, st_ino);
    PRINT_FIELD_SIZE(st, st_mode);
    PRINT_FIELD_SIZE(st, st_nlink);
    PRINT_FIELD_SIZE(st, st_uid);
    PRINT_FIELD_SIZE(st, st_gid);
    PRINT_FIELD_SIZE(st, st_rdev);
    PRINT_FIELD_SIZE(st, st_size);
    PRINT_FIELD_SIZE(st, st_atime);
    PRINT_FIELD_SIZE(st, st_mtime);
    PRINT_FIELD_SIZE(st, st_ctime);
    PRINT_FIELD_SIZE(st, st_blksize);
    PRINT_FIELD_SIZE(st, st_blocks);
    */

    cout << "Total size of stat object is " << sizeof(st) << endl;
    return 0;
}

