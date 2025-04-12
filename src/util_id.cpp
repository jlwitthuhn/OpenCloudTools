#include "util_id.h"

#include <cstring>
#include <limits>
#include <random>

#include <QByteArray>

#include "assert.h"

static std::default_random_engine engine{ std::random_device{}() };
static std::uniform_int_distribution<int> dist{ 0, std::numeric_limits<char>::max() };

static char generate_char()
{
	return static_cast<char>(dist(engine));
}

RandomId128::RandomId128()
{
	id.fill(0);
	for (char& this_byte : id)
	{
		this_byte = generate_char();
	}
}

RandomId128::RandomId128(const QByteArray& q_id)
{
	id.fill(0);
	OCTASSERT(static_cast<size_t>(q_id.size()) == id.size());
	std::memcpy(id.data(), q_id.data(), id.size());
}

bool RandomId128::operator==(const RandomId128& other) const
{
	for (size_t i = 0; i < id.size(); i++)
	{
		if (id[i] != other.id[i])
		{
			return false;
		}
	}
	return true;
}

bool RandomId128::operator<(const RandomId128& other) const
{
	for (size_t i = 0; i < id.size(); i++)
	{
		if (id[i] < other.id[i])
		{
			return true;
		}
		else if (other.id[i] < id[i])
		{
			return false;
		}
	}
	return false;
}

QByteArray RandomId128::as_q_byte_array() const
{
	QByteArray result(static_cast<QByteArray::size_type>(id.size()), '\0');
	std::memcpy(result.data(), id.data(), id.size());
	return result;
}
