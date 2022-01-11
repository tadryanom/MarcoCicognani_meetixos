/**
 * @brief
 * This file is part of the MeetiX Operating System.
 * Copyright (c) 2017-2021, Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @developers
 * Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @license
 * GNU General Public License version 3
 */

#pragma once

#include <TC/Collections/CommonIterator.hh>
#include <TC/Collections/Vector.hh>
#include <TC/Functional/Option.hh>
#include <TC/Std/Exchange.hh>
#include <TC/Std/Forward.hh>
#include <TC/Std/InitializerList.hh>
#include <TC/Std/Move.hh>
#include <TC/Std/New.hh>
#include <TC/Std/NullPtr.hh>
#include <TC/Std/Swap.hh>
#include <TC/Tags/Adopt.hh>
#include <TC/Tags/Copy.hh>
#include <TC/Tags/None.hh>
#include <TC/Tags/Wrap.hh>
#include <TC/Traits/AddConst.hh>
#include <TC/Traits/AddVolatile.hh>
#include <TC/Traits/Conditional.hh>
#include <TC/Traits/Constant.hh>
#include <TC/Traits/CopyConst.hh>
#include <TC/Traits/EnableIf.hh>
#include <TC/Traits/FalseType.hh>
#include <TC/Traits/First.hh>
#include <TC/Traits/In.hh>
#include <TC/Traits/IndexOf.hh>
#include <TC/Traits/IsBaseOf.hh>
#include <TC/Traits/IsClass.hh>
#include <TC/Traits/IsConst.hh>
#include <TC/Traits/IsFloatingPoint.hh>
#include <TC/Traits/IsFunction.hh>
#include <TC/Traits/IsIntegral.hh>
#include <TC/Traits/IsLValue.hh>
#include <TC/Traits/IsPointer.hh>
#include <TC/Traits/IsRValue.hh>
#include <TC/Traits/IsSame.hh>
#include <TC/Traits/IsUnion.hh>
#include <TC/Traits/IsVoid.hh>
#include <TC/Traits/MakeSigned.hh>
#include <TC/Traits/MakeUnsigned.hh>
#include <TC/Traits/Max.hh>
#include <TC/Traits/RemoveConst.hh>
#include <TC/Traits/RemoveConstVolatile.hh>
#include <TC/Traits/RemovePointer.hh>
#include <TC/Traits/RemoveReference.hh>
#include <TC/Traits/RemoveVolatile.hh>
#include <TC/Traits/TrueType.hh>