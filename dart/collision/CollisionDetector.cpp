/*
 * Copyright (c) 2013-2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Jeongseok Lee <jslee02@gmail.com>,
 *            Tobias Kunz <tobias@gatech.edu>
 *
 * Georgia Tech Graphics Lab and Humanoid Robotics Lab
 *
 * Directed by Prof. C. Karen Liu and Prof. Mike Stilman
 * <karenliu@cc.gatech.edu> <mstilman@cc.gatech.edu>
 *
 * This file is provided under the following "BSD-style" License:
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 */

#include "dart/collision/CollisionDetector.h"

#include <algorithm>

#include "dart/common/Console.h"
#include "dart/collision/CollisionObject.h"
#include "dart/collision/CollisionGroup.h"
#include "dart/dynamics/BodyNode.h"
#include "dart/dynamics/Skeleton.h"

namespace dart {
namespace collision {

//==============================================================================
std::shared_ptr<CollisionGroup>
CollisionDetector::createCollisionGroupAsSharedPtr()
{
  return std::shared_ptr<CollisionGroup>(createCollisionGroup().release());
}

//==============================================================================
std::shared_ptr<CollisionObject> CollisionDetector::claimCollisionObject(
    const dynamics::ShapeFrame* shapeFrame)
{
  if (!mCollisionObjectManager)
    mCollisionObjectManager.reset(new NoneSharingCollisionObjectManager(this));

  return mCollisionObjectManager->claimCollisionObject(shapeFrame);
}

//==============================================================================
void CollisionDetector::notifyCollisionObjectDestorying(
    CollisionObject* /*object*/)
{
  // Do nothing
}

//==============================================================================
CollisionDetector::CollisionObjectManager::CollisionObjectManager(
    CollisionDetector* cd)
  : mCollisionDetector(cd)
{
  assert(cd);
}

//==============================================================================
CollisionDetector::
NoneSharingCollisionObjectManager::NoneSharingCollisionObjectManager(
    CollisionDetector* cd)
  : CollisionDetector::CollisionObjectManager(cd),
    mCollisionObjectDeleter(this)
{
  // Do nothing
}

//==============================================================================
std::shared_ptr<CollisionObject>
CollisionDetector::NoneSharingCollisionObjectManager::claimCollisionObject(
    const dynamics::ShapeFrame* shapeFrame)
{
  auto uniqueObject = mCollisionDetector->createCollisionObject(shapeFrame);
  auto sharedObject = std::shared_ptr<CollisionObject>(
        uniqueObject.release(), mCollisionObjectDeleter);

  return sharedObject;
}

//==============================================================================
CollisionDetector::NoneSharingCollisionObjectManager::
CollisionObjectDeleter::CollisionObjectDeleter(
    NoneSharingCollisionObjectManager* mgr)
  : mCollisionObjectManager(mgr)
{
  assert(mgr);
}

//==============================================================================
void
CollisionDetector::NoneSharingCollisionObjectManager
::CollisionObjectDeleter::operator()(CollisionObject* object) const
{
  mCollisionObjectManager->mCollisionDetector->notifyCollisionObjectDestorying(
        object);

  delete object;
}

//==============================================================================
CollisionDetector::
SharingCollisionObjectManager::SharingCollisionObjectManager(
    CollisionDetector* cd)
  : CollisionDetector::CollisionObjectManager(cd),
    mCollisionObjectDeleter(this)
{
  // Do nothing
}

//==============================================================================
CollisionDetector::
SharingCollisionObjectManager::~SharingCollisionObjectManager()
{
  assert(mCollisionObjectMap.empty());
}

//==============================================================================
std::shared_ptr<CollisionObject>
CollisionDetector::SharingCollisionObjectManager::claimCollisionObject(
    const dynamics::ShapeFrame* shapeFrame)
{
  const auto search = mCollisionObjectMap.find(shapeFrame);

  if (mCollisionObjectMap.end() != search)
  {
    const auto& collObj = search->second;
    assert(collObj.lock());
    // Ensure all the collision object in the map should be alive pointers.

    return collObj.lock();
  }

  auto uniqueObject = mCollisionDetector->createCollisionObject(shapeFrame);
  auto sharedObject = std::shared_ptr<CollisionObject>(
        uniqueObject.release(), mCollisionObjectDeleter);

  mCollisionObjectMap[shapeFrame] = sharedObject;

  return sharedObject;
}

//==============================================================================
CollisionDetector::SharingCollisionObjectManager::
CollisionObjectDeleter::CollisionObjectDeleter(
    SharingCollisionObjectManager* mgr)
  : mCollisionObjectManager(mgr)
{
  assert(mgr);
}

//==============================================================================
void
CollisionDetector::SharingCollisionObjectManager
::CollisionObjectDeleter::operator()(CollisionObject* object) const
{
  mCollisionObjectManager->mCollisionDetector->notifyCollisionObjectDestorying(
        object);
  mCollisionObjectManager->mCollisionObjectMap.erase(object->getShapeFrame());

  delete object;
}

}  // namespace collision
}  // namespace dart
