#ifndef POWERSET_H
#define POWERSET_H

#include "extendedset.h"
#include <list>
#include <iostream>

namespace PnsTools {

template <typename BaseSetType>
class PowerSet
{
	BaseSetType mBaseSet;
public:
	class iterator
	{
		using listItemType=typename BaseSetType::iterator;
		using listType=std::list<listItemType>;
		listType mCurrentSet;
		const BaseSetType *mBaseSet=nullptr;
		friend class PowerSet;

	public:
		iterator(){}
		iterator &operator++(){
//			std::cout << "increase" << std::endl;
			if (mCurrentSet.size()==0) // start first level
			{
				mCurrentSet.push_back(mBaseSet->begin());
				return *this;
			}
			auto currentSetIt=mCurrentSet.rbegin();
			++(*currentSetIt);
			if (*currentSetIt == mBaseSet->end())
			{
				--(*currentSetIt);
				auto currentItValue=*currentSetIt;
				while (currentSetIt!=mCurrentSet.rend() && *currentSetIt==currentItValue)
				{
					++currentSetIt;
					--currentItValue;
				}
				if (currentSetIt==mCurrentSet.rend()){ // new level
					if (mCurrentSet.size()==mBaseSet->size()){ // reached the end
						mCurrentSet.clear();
						mCurrentSet.push_back(mBaseSet->end());
					}
					else{
						int counter=mCurrentSet.size();
						mCurrentSet.clear();
						currentItValue=mBaseSet->begin();
						for (int i=0; i<=counter; i++)
							mCurrentSet.push_back(currentItValue++);
					}
				}
				else{
					currentItValue=*currentSetIt;
//					std::cout << "normal with " << *currentItValue <<  std::endl;
					while (currentSetIt!=mCurrentSet.rbegin()){
//						std::cout << "step" <<  std::endl;
						*currentSetIt = ++currentItValue;
						--currentSetIt;
					}
					*currentSetIt = ++currentItValue;
				}
			}
			return *this;
		}
		bool operator!=(const iterator &other) const{
			return this->mBaseSet!=other.mBaseSet || this->mCurrentSet!=other.mCurrentSet;
		}
		BaseSetType operator*() const{
			BaseSetType currentSet;
			for (auto it : mCurrentSet)
				currentSet.insert(*it);
			return currentSet;
		}
	};

	PowerSet(const BaseSetType &baseSet):
		mBaseSet(baseSet)
	{}

	iterator begin() const{
		iterator returnIt;
		returnIt.mBaseSet=&this->mBaseSet;
		return returnIt;
	}
	iterator end() const{
		iterator returnIt;
		returnIt.mBaseSet=&this->mBaseSet;
		returnIt.mCurrentSet.push_back(this->mBaseSet.end());
		return returnIt;
	}
};

} // namespace PnsTools

#endif // POWERSET_H
