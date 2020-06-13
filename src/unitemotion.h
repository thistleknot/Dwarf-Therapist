/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef UNITEMOTION_H
#define UNITEMOTION_H

#include <QObject>
#include "global_enums.h"
#include "utils.h"
#include "dftime.h"

class DFInstance;

class UnitEmotion : public QObject
{
    Q_OBJECT

private:
    VIRTADDR m_address;
    EMOTION_TYPE m_eType;
    int m_thought_id;
    int m_sub_id;
    df_time m_time;
    QString m_desc;
    QString m_desc_colored;
    int m_count;
    int m_strength;
    float m_effect;
    int m_total_effect;
    short m_intensifier;
    int m_optional_level; //used for quality, building base wealth, maybe other things
    QString m_compare_id; //id/name used to compare in addition to thought/emotion

public:
    UnitEmotion(QObject *parent = 0);
    UnitEmotion(VIRTADDR addr, DFInstance *df, QObject *parent = 0);

    EMOTION_TYPE get_emotion_type() const {return m_eType;}
    int get_thought_id() const {return m_thought_id;}
    int get_sub_id() const {return m_sub_id;}
    int get_optional_level() const {return m_optional_level;}
    QString get_desc(bool colored = true);
    df_time get_time() const {return m_time;}
    int get_count() const {return m_count;}
    void increment_count() {m_count++;}
    int set_effect(int stress_vuln);
    short get_stress_effect() const;

    bool operator==(const UnitEmotion &other) const {
        if(this == &other)
            return true;
        if(this->m_thought_id == other.m_thought_id && this->m_eType == other.m_eType &&
                (this->m_optional_level != -1 && other.m_optional_level != -1 && this->m_optional_level == other.m_optional_level))
            return true;

        return false;
    }

    bool equals(const UnitEmotion &);

};

#endif // UNITEMOTION_H
