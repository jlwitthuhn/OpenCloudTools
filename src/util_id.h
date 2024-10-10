#pragma once

#include <array>

class QByteArray;

class RandomId128
{
public:
	static const size_t LENGTH = 16;

	RandomId128();
	RandomId128(QByteArray q_id);

	bool operator==(const RandomId128& other) const;
	bool operator<(const RandomId128& other) const;

	QByteArray as_q_byte_array() const;
private:

	std::array<char, LENGTH> id;
};
