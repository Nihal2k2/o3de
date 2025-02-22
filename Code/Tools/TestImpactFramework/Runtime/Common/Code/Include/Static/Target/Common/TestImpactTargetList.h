/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Target/Common/TestImpactTargetException.h>

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/optional.h>
#include <AzCore/std/sort.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/string/string.h>

#include <algorithm>

namespace TestImpact
{
    //! Container for unique set of sorted target types.
    //! @tparam Target The specialized target type.
    template<typename Target>
    class TargetList
    {
    public:
        using TargetType = Target;

        TargetList(AZStd::vector<AZStd::unique_ptr<typename Target::Descriptor>>&& descriptors);

        //! Returns the targets in the collection.
        const AZStd::vector<Target>& GetTargets() const;

        //! Returns the target with the specified name.
        const Target* GetTarget(const AZStd::string& name) const;

        //! Returns the target with the specified name or throws if target not found.
        const Target* GetTargetOrThrow(const AZStd::string& name) const;

        //! Returns true if the specified target is in the list, otherwise false.
        bool HasTarget(const AZStd::string& name) const;

        //! Returns the number of targets in the list.
        size_t GetNumTargets() const;

    private:
        AZStd::vector<Target> m_targets;
    };

    template<typename Target>
    TargetList<Target>::TargetList(AZStd::vector<AZStd::unique_ptr<typename Target::Descriptor>>&& descriptors)
    {
        AZ_TestImpact_Eval(!descriptors.empty(), TargetException, "Target list is empty");

        AZStd::sort(
            descriptors.begin(), descriptors.end(),
            [](const AZStd::unique_ptr<typename Target::Descriptor>& lhs, const AZStd::unique_ptr<typename Target::Descriptor>& rhs)
        {
                return lhs->m_name < rhs->m_name;
        });

        const auto duplicateElement = AZStd::adjacent_find(
            descriptors.begin(), descriptors.end(),
            [](const AZStd::unique_ptr<typename Target::Descriptor>& lhs, const AZStd::unique_ptr<typename Target::Descriptor>& rhs)
        {
                return lhs->m_name == rhs->m_name;
        });

        AZ_TestImpact_Eval(duplicateElement == descriptors.end(), TargetException, "Target list contains duplicate targets");

        m_targets.reserve(descriptors.size());
        for (auto&& descriptor : descriptors)
        {
            m_targets.emplace_back(Target(AZStd::move(descriptor)));
        }
    }

    template<typename Target>
    const AZStd::vector<Target>& TargetList<Target>::GetTargets() const
    {
        return m_targets;
    }

    template<typename Target>
    size_t TargetList<Target>::GetNumTargets() const
    {
        return m_targets.size();
    }

    template<typename Target>
    const Target* TargetList<Target>::GetTarget(const AZStd::string& name) const
    {
        struct TargetComparator
        {
            bool operator()(const Target& target, const AZStd::string& name) const
            {
                return target.GetName() < name;
            }

            bool operator()(const AZStd::string& name, const Target& target) const
            {
                return name < target.GetName();
            }
        };

        const auto targetRange = std::equal_range(m_targets.begin(), m_targets.end(), name, TargetComparator{});

        if (targetRange.first != targetRange.second)
        {
            return targetRange.first;
        }
        else
        {
            return nullptr;
        }
    }

    template<typename Target>
    const Target* TargetList<Target>::GetTargetOrThrow(const AZStd::string& name) const
    {
        const Target* target = GetTarget(name);
        AZ_TestImpact_Eval(target, TargetException, AZStd::string::format("Couldn't find target %s", name.c_str()).c_str());
        return target;
    }

    template<typename Target>
    bool TargetList<Target>::HasTarget(const AZStd::string& name) const
    {
        return GetTarget(name) != nullptr;
    }
} // namespace TestImpact
