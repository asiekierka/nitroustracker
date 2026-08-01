/*====================================================================
Copyright 2006 Tobias Weyand

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
======================================================================*/

#include "tobkit/radiobutton.h"

using namespace tobkit;

RadioButton::RadioButtonGroup::RadioButtonGroup()
{
	onChange = 0;
}

void RadioButton::RadioButtonGroup::add(RadioButton *rb)
{
	rbvec.push_back(rb);
}

void RadioButton::RadioButtonGroup::pushed(RadioButton *rb)
{
	u8 counter = 0, pos = 0;
	std::vector<RadioButton *>::iterator rbit;

	for (rbit = rbvec.begin(); rbit != rbvec.end(); ++rbit) {
		if (*rbit != rb) {
			(*rbit)->setActive(false);
		} else {
			(*rbit)->setActive(true);
			pos = counter;
		}
		counter++;
	}

	if (onChange) {
		onChange(pos);
	}
}

void RadioButton::RadioButtonGroup::setActive(u8 idx)
{
	pushed(rbvec.at(idx));
}

void RadioButton::RadioButtonGroup::registerChangeCallback(
    void (*onChange_)(int))
{
	onChange = onChange_;
}
