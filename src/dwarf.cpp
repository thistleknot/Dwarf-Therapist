#include <QVector>
#include "dwarf.h"
#include "dfinstance.h"
#include "skill.h"
#include "gamedatareader.h"

Dwarf::Dwarf(DFInstance *df, int address, QObject *parent)
	: QObject(parent)
	, m_df(df)
	, m_address(address)
	, m_labors(new uchar[102])
{
	refresh_data();
}

void Dwarf::refresh_data() {
	GameDataReader *gdr = GameDataReader::ptr();
	uint bytes_read = 0;

	m_first_name = m_df->read_string(m_address + gdr->get_dwarf_offset("first_name"));
	if (m_first_name.size() > 1)
		m_first_name[0] = m_first_name[0].toUpper();
	
	m_id = m_df->read_int32(m_address + gdr->get_dwarf_offset("id"), bytes_read);
	m_nick_name = m_df->read_string(m_address + gdr->get_dwarf_offset("nick_name"));
    m_last_name = read_last_name(m_address + gdr->get_dwarf_offset("last_name"));
	m_custom_profession = m_df->read_string(m_address + gdr->get_dwarf_offset("custom_profession"));
	m_race_id = m_df->read_int32(m_address + gdr->get_dwarf_offset("race"), bytes_read);
    m_skills = read_skills(m_address + gdr->get_dwarf_offset("skills"));
	m_profession = read_professtion(m_address + gdr->get_dwarf_offset("profession"));
	m_strength = m_df->read_int32(m_address + gdr->get_dwarf_offset("strength"), bytes_read);
	m_toughness = m_df->read_int32(m_address + gdr->get_dwarf_offset("toughness"), bytes_read);
	m_agility = m_df->read_int32(m_address + gdr->get_dwarf_offset("agility"), bytes_read);
	read_labors(m_address + gdr->get_dwarf_offset("labors"));
}

Dwarf::~Dwarf() {
	delete[] m_labors;
}

Dwarf *Dwarf::get_dwarf(DFInstance *df, int address) {
	GameDataReader *gdr = GameDataReader::ptr();
	uint bytes_read = 0;
	if ((df->read_int32(address + gdr->get_dwarf_offset("flags1"), bytes_read) & gdr->get_int_for_key("flags/flags1.invalidate")) > 0) {
		return 0;
	}
	if ((df->read_int32(address + gdr->get_dwarf_offset("flags2"), bytes_read) & gdr->get_int_for_key("flags/flags2.invalidate")) > 0) {
		return 0;
	}
	if ((df->read_int32(address + gdr->get_dwarf_offset("race"), bytes_read)) != 166) {
		return 0;
	}
	//if( memoryAccess.ReadInt32( address + memoryLayout["Creature.Race"] ) != actualDwarfRaceId )
	//	return false;
	return new Dwarf(df, address, df);
}

QString Dwarf::to_string() {
	return QString("%1, %2 STR:%3 AGI:%4 TOU:%5").arg(nice_name(), m_profession).arg(m_strength).arg(m_agility).arg(m_toughness);
}

QString Dwarf::nice_name() {
	if (m_nick_name.isEmpty()) {
		return QString("%1 %2").arg(m_first_name, m_last_name);
	} else {
		return QString("\"%1\" %2 ").arg(m_nick_name, m_last_name);
	}
}

QString Dwarf::read_last_name(int address) {
    // TODO: move to config
	GameDataReader *gdr = GameDataReader::ptr();
    int word_table = gdr->get_offset("word_table");
    uint bytes_read = 0;
    int actual_lang_table = m_df->read_int32(gdr->get_address("language_vector") + m_df->get_memory_correction() + 4, bytes_read);
    int translations_ptr = m_df->read_int32(gdr->get_address("translation_vector") + m_df->get_memory_correction() + 4, bytes_read);
    int translation_ptr = m_df->read_int32(translations_ptr, bytes_read);
    int actual_dwarf_translation_table = m_df->read_int32(translation_ptr + word_table, bytes_read);

    QString out;

	for (int i = 0; i < 7; i++) {
        int word = m_df->read_int32(address + i * 4, bytes_read);
        if(bytes_read == 0 || word == -1)
			break;
        //Q_ASSERT(word < 10000);
        int addr = m_df->read_int32(actual_dwarf_translation_table + word * 4, bytes_read);
        out += m_df->read_string(addr);
	}
    if (out.size() > 1) {
        out[0] = out[0].toUpper();
    }
    return out;
}


QVector<Skill> Dwarf::read_skills(int address) {
    QVector<Skill> skills(0);
    uint bytes_read = 0;
	foreach(int addr, m_df->enumerate_vector(address)) {
		/*uchar buff[6];
		memset(buff, 0, 90);
		bytes_read = m_df->read_raw(addr-30, 60, &buff[0]);
		//short btype = buff[0] + (buff[1] << 8);
		short bexp = (short)((int)(buff[9] << 8) + (int)buff[8]);
		//short brat = buff[4] + (buff[5] << 8);
		*/

        short type = m_df->read_short(addr, bytes_read);
        short experience = m_df->read_short(addr + 2, bytes_read);
        short rating = m_df->read_short(addr + 4, bytes_read);
		Skill s(type, experience, rating);
        skills.append(s);
    }
	return skills;
}

short Dwarf::get_rating_for_skill(int labor_id) {
	GameDataReader *gdr = GameDataReader::ptr();
	int skill_id = gdr->get_int_for_key("labor_to_skill/" + QString::number(labor_id), 10);
	if (skill_id == -1) { // not found
		return 0;
	}
	foreach(Skill s, m_skills) {
		if (s.id() == skill_id) {
			return s.rating();
		}
	}
	return 0;
}

QString Dwarf::read_professtion(int address) {
	if (!m_custom_profession.isEmpty()) {
		return m_custom_profession; 
	}
	
	char buffer[1];
	int bytes_read = m_df->read_raw(address, 1, &buffer[0]);
	return GameDataReader::ptr()->get_profession_name((int)buffer[0]);
}

void Dwarf::read_labors(int address) {
	//uchar buf[400];
	//memset(buf, 0, 400);
	//int bytes_read = m_df->read_raw(address, 102, &buf);
	memset(m_labors, 0, 102);
	int bytes_read = m_df->read_raw(address, 102, m_labors);
}

bool Dwarf::toggle_labor(int labor_id) {
	if (m_labors[labor_id]) {
		m_labors[labor_id] = 0;
	} else {
		m_labors[labor_id] = 1;
	}
	//TODO: if "write_immediate"
	GameDataReader *gdr = GameDataReader::ptr();
	int bytes_written = m_df->write_raw(m_address + gdr->get_dwarf_offset("labors"), 102, m_labors);
	return bytes_written == 102;
}