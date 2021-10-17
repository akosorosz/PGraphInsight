#ifndef EXTENDEDSET_H
#define EXTENDEDSET_H

#include <set>
#include <algorithm>

namespace PnsTools {

template <typename DataType, typename CompareType = std::less<DataType>>
class ExtendedSet : public std::set<DataType,CompareType>
{
public:
	using std::set<DataType,CompareType>::set;
	using iterator=typename std::set<DataType,CompareType>::iterator;
	using const_iterator=typename std::set<DataType,CompareType>::const_iterator;
	using reverse_iterator=typename std::set<DataType,CompareType>::reverse_iterator;
	using const_reverse_iterator=typename std::set<DataType,CompareType>::const_reverse_iterator;

	const DataType &front() const{
		return *this->begin();
	}
	const DataType &back() const{
		return *this->rbegin();
	}

	ExtendedSet &operator+=(const DataType &item){
		this->insert(item);
		return *this;
	}
	ExtendedSet &operator-=(const DataType &item){
		this->erase(item);
		return *this;
	}
	ExtendedSet operator+(const DataType &item) const{
		ExtendedSet result(*this);
		result+=item;
		return result;
	}
	ExtendedSet operator-(const DataType &item) const{
		ExtendedSet result(*this);
		result-=item;
		return result;
	}

	ExtendedSet operator &(const ExtendedSet &other) const{
		ExtendedSet result;
		std::set_intersection(this->begin(), this->end(), other.begin(), other.end(), std::inserter(result, result.end()), CompareType());
		return result;
	}
	ExtendedSet operator |(const ExtendedSet &other) const{
		ExtendedSet result;
		std::set_union(this->begin(), this->end(), other.begin(), other.end(), std::inserter(result, result.end()), CompareType());
		return result;
	}
	ExtendedSet operator /(const ExtendedSet &other) const{
		ExtendedSet result;
		std::set_difference(this->begin(), this->end(), other.begin(), other.end(), std::inserter(result, result.end()), CompareType());
		return result;
	}
	ExtendedSet &operator &=(const ExtendedSet &other)
	{
		*this=*this & other;
		return *this;
	}
	ExtendedSet &operator |=(const ExtendedSet &other)
	{
		*this=*this | other;
		return *this;
	}
	ExtendedSet &operator /=(const ExtendedSet &other)
	{
		*this=*this / other;
		return *this;
	}
	ExtendedSet &operator+=(const ExtendedSet &other){
		return *this |= other;
	}
	ExtendedSet &operator-=(const ExtendedSet &other){
		return *this /= other;
	}
	ExtendedSet operator+(const ExtendedSet &other) const{
		return *this | other;
	}
	ExtendedSet operator-(const ExtendedSet &other) const{
		return *this / other;
	}

	// C++20 has contains already, but we only use C++17 here
	bool contains(const DataType &item) const{
		return this->find(item)!=this->end();
	}
	bool isSubsetOf(const ExtendedSet &other) const{
		if (this->size()>other.size()) return false;
		return (*this/other).size()==0;
	}
	bool isRealSubsetOf(const ExtendedSet &other) const{
		if (this->size()>=other.size()) return false;
		return (*this/other).size()==0;
	}
	bool operator<(const ExtendedSet &other) const{
		return this->isRealSubsetOf(other);
	}
	bool operator<=(const ExtendedSet &other) const{
		return this->isSubsetOf(other);
	}
	bool operator>(const ExtendedSet &other) const{
		return other.isRealSubsetOf(*this);
	}
	bool operator>=(const ExtendedSet &other) const{
		return other.isSubsetOf(*this);
	}
};

template <typename DataType, typename CompareType = std::less<DataType>>
bool operator<(const DataType &item, const ExtendedSet<DataType,CompareType> &eset){
	return eset.contains(item);
}

} // namespace PnsTools

#endif // EXTENDEDSET_H
