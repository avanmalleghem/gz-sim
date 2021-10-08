/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <vector>

#include <ignition/plugin/Register.hh>
#include <ignition/transport/Node.hh>

#include <ignition/common/Profiler.hh>

#include <sdf/Element.hh>

#include "ignition/gazebo/components/DetachableJoint.hh"
#include "ignition/gazebo/components/Link.hh"
#include "ignition/gazebo/components/Model.hh"
#include "ignition/gazebo/components/Name.hh"
#include "ignition/gazebo/components/ParentEntity.hh"
#include "ignition/gazebo/components/Pose.hh"
#include "ignition/gazebo/Model.hh"
#include "ignition/gazebo/Util.hh"

#include "DetachableJoint.hh"

using namespace ignition;
using namespace gazebo;
using namespace systems;

/////////////////////////////////////////////////
void DetachableJoint::Configure(const Entity &_entity,
               const std::shared_ptr<const sdf::Element> &_sdf,
               EntityComponentManager &_ecm,
               EventManager &/*_eventMgr*/)
{
  this->model = Model(_entity);
  if (!this->model.Valid(_ecm))
  {
    ignerr << "DetachableJoint should be attached to a model entity. "
           << "Failed to initialize." << std::endl;
    return;
  }

  if (_sdf->HasElement("parent_link"))
  {
    auto parentLinkName = _sdf->Get<std::string>("parent_link");
    this->parentLinkEntity = this->model.LinkByName(_ecm, parentLinkName);
    if (kNullEntity == this->parentLinkEntity)
    {
      ignerr << "Link with name " << parentLinkName
             << " not found in model " << this->model.Name(_ecm)
             << ". Make sure the parameter 'parent_link' has the "
             << "correct value. Failed to initialize.\n";
      return;
    }
  }
  else
  {
    ignerr << "'parent_link' is a required parameter for DetachableJoint. "
              "Failed to initialize.\n";
    return;
  }

  if (_sdf->HasElement("child_model"))
  {
    this->childModelName = _sdf->Get<std::string>("child_model");
  }
  else
  {
    ignerr << "'child_model' is a required parameter for DetachableJoint."
              "Failed to initialize.\n";
    return;
  }

  if (_sdf->HasElement("child_link"))
  {
    this->childLinkName = _sdf->Get<std::string>("child_link");
  }
  else
  {
    ignerr << "'child_link' is a required parameter for DetachableJoint."
              "Failed to initialize.\n";
    return;
  }

  // Setup detach topic
  std::vector<std::string> topics;
  if (_sdf->HasElement("topic"))
  {
    topics.push_back(_sdf->Get<std::string>("topic"));
  }
  topics.push_back("/model/" + this->model.Name(_ecm) +
      "/detachable_joint/detach");
  this->topic = validTopic(topics);

  this->suppressChildWarning =
      _sdf->Get<bool>("suppress_child_warning", this->suppressChildWarning)
          .first;

  this->validConfig = true;
}

//////////////////////////////////////////////////
void DetachableJoint::PreUpdate(
  const ignition::gazebo::UpdateInfo &/*_info*/,
  ignition::gazebo::EntityComponentManager &_ecm)
{
  IGN_PROFILE("DetachableJoint::PreUpdate");
  if (this->validConfig && !this->initialized)
  {
    // Look for the child model and link
    Entity modelEntity{kNullEntity};

    if ("__model__" == this->childModelName)
    {
      modelEntity = this->model.Entity();
    }
    else
    {
      auto candidateEntities = _ecm.EntitiesByComponents(
          components::Model(), components::Name(this->childModelName));

      if (candidateEntities.size() == 1)
      {
        // If there is one entity select that entity itself
        modelEntity = candidateEntities[0];
      }
      else
      {
        // If there is more than one entity with the same name, look for the
        // entity with a parent component.
        for(auto entity : candidateEntities)
        {
          // TODO(arjo): do we want to support grand children?
          // what about if there are multiple children with the same name?
          auto models = this->model.Models(_ecm);

          for (auto model : models)
          {
            if (model == entity)
            {
              modelEntity = entity;
            }
          }
        }
      }
    }
    if (kNullEntity != modelEntity)
    {
      this->childLinkEntity = _ecm.EntityByComponents(
          components::Link(), components::ParentEntity(modelEntity),
          components::Name(this->childLinkName));

      if (kNullEntity != this->childLinkEntity)
      {
        // Attach the models
        // We do this by creating a detachable joint entity.
        this->detachableJointEntity = _ecm.CreateEntity();

        _ecm.CreateComponent(
            this->detachableJointEntity,
            components::DetachableJoint({this->parentLinkEntity,
                                         this->childLinkEntity, "fixed"}));

        this->node.Subscribe(
            this->topic, &DetachableJoint::OnDetachRequest, this);

        ignmsg << "DetachableJoint subscribing to messages on "
               << "[" << this->topic << "]" << std::endl;

        this->initialized = true;
      }
      else
      {
        ignwarn << "Child Link " << this->childLinkName
                << " could not be found.\n";
      }
    }
    else if (!this->suppressChildWarning)
    {
      ignwarn << "Child Model " << this->childModelName
              << " could not be found.\n";
    }
  }

  if (this->initialized)
  {
    if (this->detachRequested && (kNullEntity != this->detachableJointEntity))
    {
      // Detach the models
      igndbg << "Removing entity: " << this->detachableJointEntity << std::endl;
      _ecm.RequestRemoveEntity(this->detachableJointEntity);
      this->detachableJointEntity = kNullEntity;
      this->detachRequested = false;
    }
  }
}

//////////////////////////////////////////////////
void DetachableJoint::OnDetachRequest(const msgs::Empty &)
{
  this->detachRequested = true;
}

IGNITION_ADD_PLUGIN(DetachableJoint,
                    ignition::gazebo::System,
                    DetachableJoint::ISystemConfigure,
                    DetachableJoint::ISystemPreUpdate)

IGNITION_ADD_PLUGIN_ALIAS(DetachableJoint,
  "ignition::gazebo::systems::DetachableJoint")
