#ifndef EPIWORLD_PERSON_MEAT_HPP
#define EPIWORLD_PERSON_MEAT_HPP

// template<typename Ta>
// inline bool IN(Ta & a, std::vector< Ta > & b);

#define CHECK_COALESCE_(proposed_, virus_tool_, alt_) \
    if (static_cast<int>(proposed_) == -99) {\
        if (static_cast<int>(virus_tool_) == -99) \
            (proposed_) = (alt_);\
        else (proposed_) = (virus_tool_);}

// To large to add directly here
#include "agent-actions-meat.hpp"

template<typename TSeq>
inline Agent<TSeq>::Agent() {}

template<typename TSeq>
inline Agent<TSeq>::Agent(Agent<TSeq> && p) :
    neighbors(std::move(p.neighbors)),
    entities(std::move(p.entities)),
    entities_locations(std::move(p.entities_locations)),
    n_entities(p.n_entities),
    status(p.status),
    status_prev(p.status_prev), 
    status_last_changed(p.status_last_changed),
    id(p.id),
    viruses(std::move(p.viruses)),  /// Needs to be adjusted
    n_viruses(p.n_viruses),
    tools(std::move(p.tools)), /// Needs to be adjusted
    n_tools(p.n_tools),
    add_virus_(std::move(p.add_virus_)),
    add_tool_(std::move(p.add_tool_)),
    add_entity_(std::move(p.add_entity_)),
    rm_virus_(std::move(p.rm_virus_)),
    rm_tool_(std::move(p.rm_tool_)),
    rm_entity_(std::move(p.rm_entity_)),
    action_counter(p.action_counter)
{

    status = p.status;
    id     = p.id;
    
    // Dealing with the virus

    int loc = 0;
    for (auto & v : viruses)
    {
        
        // Will create a copy of the virus, with the exeption of
        // the virus code
        v->agent = this;
        v->agent_idx = loc++;

    }

    loc = 0;
    for (auto & t : tools)
    {
        
        // Will create a copy of the virus, with the exeption of
        // the virus code
        t->agent     = this;
        t->agent_idx = loc++;

    }
    
}

template<typename TSeq>
inline Agent<TSeq>::Agent(const Agent<TSeq> & p) :
    neighbors(0u),
    entities(0u),
    entities_locations(0u),
    n_entities(0u),
    sampled_agents(0u),
    sampled_agents_n(0u),
    sampled_agents_left_n(0u),
    date_last_build_sample(-99)
{

    status = p.status;
    id     = p.id;
    
    // Dealing with the virus
    viruses.resize(p.get_n_viruses());
    n_viruses = viruses.size();
    for (size_t i = 0u; i < viruses.size(); ++i)
    {

        // Will create a copy of the virus, with the exeption of
        // the virus code
        viruses[i] = std::make_shared<Virus<TSeq>>(*p.viruses[i]);
        viruses[i]->agent = this;
        viruses[i]->agent_idx = i;

    }

    tools.resize(p.get_n_tools());
    for (size_t i = 0u; i < tools.size(); ++i)
    {
        
        // Will create a copy of the virus, with the exeption of
        // the virus code
        tools[i] = std::make_shared<Tool<TSeq>>(*p.tools[i]);
        tools[i]->agent = this;
        tools[i]->agent_idx = i;

    }

    add_virus_ = p.add_virus_;
    add_tool_  = p.add_tool_;
    rm_virus_  = p.rm_virus_;
    rm_tool_   = p.rm_tool_;
    
}

template<typename TSeq>
inline Agent<TSeq> & Agent<TSeq>::operator=(
    const Agent<TSeq> & other_agent
) 
{

    neighbors.clear();
    entities.clear();
    entities_locations.clear();
    n_entities = 0;
    sampled_agents.clear();
    sampled_agents_n = 0;
    sampled_agents_left_n = 0;
    date_last_build_sample = -99;

    // neighbors           = other_agent.neighbors;
    // entities            = other_agent.entities;
    // entities_locations  = other_agent.entities_locations;
    // n_entities          = other_agent.n_entities;
    status              = other_agent.status;
    status_prev         = other_agent.status_prev;
    status_last_changed = other_agent.status_last_changed;
    id                  = other_agent.id;
    
    viruses             = other_agent.viruses;
    n_viruses           = other_agent.n_viruses;
    
    tools               = other_agent.tools;
    n_tools             = other_agent.n_tools;

    add_virus_          = other_agent.add_virus_;
    add_tool_           = other_agent.add_tool_;
    add_entity_         = other_agent.add_entity_;
    rm_virus_           = other_agent.rm_virus_;
    rm_tool_            = other_agent.rm_tool_;
    rm_entity_          = other_agent.rm_entity_;
    action_counter      = other_agent.action_counter;
    
    // Dealing with the virus
    for (auto & v : viruses)
    {
        
        // Will create a copy of the virus, with the exeption of
        // the virus code
        v->agent = this;
        v->agent_idx = id;

    }

    for (auto & t : tools)
    {
        
        // Will create a copy of the virus, with the exeption of
        // the virus code
        t->agent     = this;
        t->agent_idx = id;

    }

    return *this;
    
}

template<typename TSeq>
inline void Agent<TSeq>::add_tool(
    ToolPtr<TSeq> tool,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
) {

    // Checking the virus exists
    if (tool->get_id() >= static_cast<int>(model->get_db().get_n_tools()))
        throw std::range_error("The tool with id: " + std::to_string(tool->get_id()) + 
            " has not been registered. There are only " + std::to_string(model->get_n_tools()) + 
            " included in the model.");
    

    model->actions_add(
        this, nullptr, tool, nullptr, status_new, queue, add_tool_, -1, -1
        );

}

template<typename TSeq>
inline void Agent<TSeq>::add_tool(
    Tool<TSeq> tool,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{
    ToolPtr<TSeq> tool_ptr = std::make_shared< Tool<TSeq> >(tool);
    add_tool(tool_ptr, model, status_new, queue);
}

template<typename TSeq>
inline void Agent<TSeq>::add_virus(
    VirusPtr<TSeq> virus,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    // Checking the virus exists
    if (virus->get_id() >= static_cast<int>(model->get_db().get_n_variants()))
        throw std::range_error("The virus with id: " + std::to_string(virus->get_id()) + 
            " has not been registered. There are only " + std::to_string(model->get_n_variants()) + 
            " included in the model.");

    model->actions_add(
        this, virus, nullptr, nullptr, status_new, queue, add_virus_, -1, -1
        );

}

template<typename TSeq>
inline void Agent<TSeq>::add_virus(
    Virus<TSeq> virus,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{
    VirusPtr<TSeq> virus_ptr = std::make_shared< Virus<TSeq> >(virus);
    add_virus(virus_ptr, model, status_new, queue);
}

template<typename TSeq>
inline void Agent<TSeq>::add_entity(
    Entity<TSeq> & entity,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    if (model != nullptr)
    {

        model->actions_add(
            this, nullptr, nullptr, &entity, status_new, queue, add_entity_, -1, -1
        );

    }
    else // If no model is passed, then we assume that we only need to add the
         // model entity
    {

        Action<TSeq> a(
                this, nullptr, nullptr, &entity, status_new, queue, add_entity_,
                -1, -1
            );

        default_add_entity(a, model); /* passing model makes nothing */

    }

}

template<typename TSeq>
inline void Agent<TSeq>::rm_tool(
    epiworld_fast_uint tool_idx,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    if (tool_idx >= n_tools)
        throw std::range_error(
            "The Tool you want to remove is out of range. This Agent only has " +
            std::to_string(n_tools) + " tools."
        );

    model->actions_add(
        this, nullptr, tools[tool_idx], nullptr, status_new, queue, rm_tool_, -1, -1
        );

}

template<typename TSeq>
inline void Agent<TSeq>::rm_tool(
    ToolPtr<TSeq> & tool,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    if (tool->agent != this)
        throw std::logic_error("Cannot remove a virus from another agent!");

    model->actions_add(
        this, nullptr, tool, nullptr, status_new, queue, rm_tool_, -1, -1
        );

}

template<typename TSeq>
inline void Agent<TSeq>::rm_virus(
    epiworld_fast_uint virus_idx,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{
    if (virus_idx >= n_viruses)
        throw std::range_error(
            "The Virus you want to remove is out of range. This Agent only has " +
            std::to_string(n_viruses) + " viruses."
        );
    else if (n_viruses == 0u)
        throw std::logic_error(
            "There is no virus to remove here!"
        );


    model->actions_add(
        this, viruses[virus_idx], nullptr, nullptr, status_new, queue,
        default_rm_virus<TSeq>, -1, -1
        );
    
}

template<typename TSeq>
inline void Agent<TSeq>::rm_virus(
    VirusPtr<TSeq> & virus,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    if (virus->agent != this)
        throw std::logic_error("Cannot remove a virus from another agent!");

    model->actions_add(
        this, virus, nullptr, nullptr, status_new, queue,
        default_rm_virus<TSeq>, -1, -1
        );


}

template<typename TSeq>
inline void Agent<TSeq>::rm_entity(
    epiworld_fast_uint entity_idx,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    if (entity_idx >= n_entities)
        throw std::range_error(
            "The Entity you want to remove is out of range. This Agent only has " +
            std::to_string(n_entities) + " entitites."
        );
    else if (n_entities == 0u)
        throw std::logic_error(
            "There is entity to remove here!"
        );

    model->actions_add(
        this, nullptr, nullptr, entities[entity_idx], status_new, queue, 
        default_rm_entity, entities_locations[entity_idx], entity_idx
    );
}

template<typename TSeq>
inline void Agent<TSeq>::rm_entity(
    Entity<TSeq> & entity,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    // Looking for entity location in the agent
    int entity_idx = -1;
    for (size_t i = 0u; i < n_entities; ++i)
    {
        if (entities[i]->get_id() == entity->get_id())
            entity_idx = i;
    }

    if (entity_idx == -1)
        throw std::logic_error(
            "The agent " + std::to_string(id) + " is not associated with entity \"" +
            entity.get_name() + "\"."
            );


    model->actions_add(
        this, nullptr, nullptr, entities[entity_idx], status_new, queue, 
        default_rm_entity, entities_locations[entity_idx], entity_idx
    );
}

template<typename TSeq>
inline void Agent<TSeq>::rm_agent_by_virus(
    epiworld_fast_uint virus_idx,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    if (status_new == -99)
        status_new = status;

    if (virus_idx >= n_viruses)
        throw std::range_error(
            std::string("The virus trying to remove the agent is out of range. ") +
            std::string("This agent has only ") + std::to_string(n_viruses) + 
            std::string(" and you are trying to remove virus # ") +
            std::to_string(virus_idx) + std::string(".")
            );

    // Removing viruses
    for (size_t i = 0u; i < n_viruses; ++i)
    {
        if (i != virus_idx)
            rm_virus(i, model);
    }

    // Changing status to new_status
    VirusPtr<TSeq> & v = viruses[virus_idx];
    epiworld_fast_int dead_status, dead_queue;
    v->get_status(nullptr, nullptr, &dead_status);
    v->get_queue(nullptr, nullptr, &dead_queue);

    if (queue != -99)
        dead_queue = queue;

    change_status(
        model,
        // Either preserve the current status or apply a new one
        (dead_status < 0) ? status : static_cast<epiworld_fast_uint>(dead_status),

        // By default, it will be removed from the queue... unless the user
        // says the contrary!
        (dead_queue == -99) ? QueueValues::NoOne : dead_queue
    );

}

template<typename TSeq>
inline void Agent<TSeq>::rm_agent_by_virus(
    VirusPtr<TSeq> & virus,
    Model<TSeq> * model,
    epiworld_fast_int status_new,
    epiworld_fast_int queue
)
{

    if (virus->get_agent() == nullptr)
        throw std::logic_error("The virus trying to remove the agent has no host.");

    if (virus->get_agent()->id != id)
        throw std::logic_error("Viruses can only remove their hosts'.");

    rm_agent_by_virus(
        virus->agent_idx,
        model,
        status_new,
        queue
    );

}

template<typename TSeq>
inline epiworld_double Agent<TSeq>::get_susceptibility_reduction(
    VirusPtr<TSeq> v,
    Model<TSeq> * model
) {

    return model->susceptibility_reduction_mixer(this, v, model);
}

template<typename TSeq>
inline epiworld_double Agent<TSeq>::get_transmission_reduction(
    VirusPtr<TSeq> v,
    Model<TSeq> * model
) {
    return model->transmission_reduction_mixer(this, v, model);
}

template<typename TSeq>
inline epiworld_double Agent<TSeq>::get_recovery_enhancer(
    VirusPtr<TSeq> v,
    Model<TSeq> * model
) {
    return model->recovery_enhancer_mixer(this, v, model);
}

template<typename TSeq>
inline epiworld_double Agent<TSeq>::get_death_reduction(
    VirusPtr<TSeq> v,
    Model<TSeq> * model
) {
    return model->death_reduction_mixer(this, v, model);
}

template<typename TSeq>
inline int Agent<TSeq>::get_id() const
{
    return id;
}

template<typename TSeq>
inline Viruses<TSeq> Agent<TSeq>::get_viruses() {

    return Viruses<TSeq>(*this);

}

template<typename TSeq>
inline const Viruses_const<TSeq> Agent<TSeq>::get_viruses() const {

    return Viruses_const<TSeq>(*this);
    
}

template<typename TSeq>
inline VirusPtr<TSeq> & Agent<TSeq>::get_virus(int i) {
    return viruses.at(i);
}

template<typename TSeq>
inline size_t Agent<TSeq>::get_n_viruses() const noexcept
{
    return n_viruses;
}

template<typename TSeq>
inline Tools<TSeq> Agent<TSeq>::get_tools() {
    return Tools<TSeq>(*this);
}

template<typename TSeq>
inline const Tools_const<TSeq> Agent<TSeq>::get_tools() const {
    return Tools_const<TSeq>(*this);
}

template<typename TSeq>
inline ToolPtr<TSeq> & Agent<TSeq>::get_tool(int i)
{
    return tools.at(i);
}

template<typename TSeq>
inline size_t Agent<TSeq>::get_n_tools() const noexcept
{
    return n_tools;
}

template<typename TSeq>
inline void Agent<TSeq>::mutate_variant()
{

    for (auto & v : viruses)
        v->mutate();

}

template<typename TSeq>
inline void Agent<TSeq>::add_neighbor(
    Agent<TSeq> * p,
    bool check_source,
    bool check_target
) {
    // Can we find the neighbor?
    if (check_source)
    {

        bool found = false;
        for (auto & n: neighbors)    
            if (n->get_id() == p->get_id())
            {
                found = true;
                break;
            }

        if (!found)
            neighbors.push_back(p);

    } else 
        neighbors.push_back(p);

    if (check_target)
    {

        bool found = false;
        for (auto & n: p->neighbors)
            if (n->get_id() == id)
            {
                found = true;
                break;
            }

        if (!found)
            p->neighbors.push_back(this);
    
    } else 
        p->neighbors.push_back(this);
    

}

template<typename TSeq>
inline std::vector< Agent<TSeq> *> & Agent<TSeq>::get_neighbors()
{
    return neighbors;
}

template<typename TSeq>
inline void Agent<TSeq>::change_status(
    Model<TSeq> * model,
    epiworld_fast_uint new_status,
    epiworld_fast_int queue
    )
{

    model->actions_add(
        this, nullptr, nullptr, nullptr, new_status, queue, nullptr, -1, -1
    );
    
    return;

}

template<typename TSeq>
inline const epiworld_fast_uint & Agent<TSeq>::get_status() const {
    return status;
}

template<typename TSeq>
inline void Agent<TSeq>::reset()
{

    this->viruses.clear();
    n_viruses = 0u;

    this->tools.clear();
    n_tools = 0u;

    this->status = 0u;
    this->status_prev = 0u;

    this->status_last_changed = -1;
    
}

template<typename TSeq>
inline bool Agent<TSeq>::has_tool(epiworld_fast_uint t) const
{

    for (auto & tool : tools)
        if (tool->get_id() == t)
            return true;

    return false;

}

template<typename TSeq>
inline bool Agent<TSeq>::has_tool(std::string name) const
{

    for (auto & tool : tools)
        if (tool->get_name() == name)
            return true;

    return false;

}

template<typename TSeq>
inline bool Agent<TSeq>::has_virus(epiworld_fast_uint t) const
{
    for (auto & v : viruses)
        if (v->get_id() == t)
            return true;

    return false;
}

template<typename TSeq>
inline bool Agent<TSeq>::has_virus(std::string name) const
{
    
    for (auto & v : viruses)
        if (v->get_name() == name)
            return true;

    return false;

}

template<typename TSeq>
inline void Agent<TSeq>::print(
    Model<TSeq> * model,
    bool compressed
    ) const
{

    if (compressed)
    {
        printf_epiworld(
            "Agent: %i, Status: %s (%lu), Nvirus: %lu, NTools: %lu, NNeigh: %lu\n",
            id, model->status_labels[status].c_str(), status, n_viruses, n_tools, neighbors.size()
        );
    }
    else {

        printf_epiworld("Information about agent id %i\n", this->id);
        printf_epiworld("  Status       : %s (%lu)\n", model->status_labels[status].c_str(), status);
        printf_epiworld("  Virus count  : %lu\n", n_viruses);
        printf_epiworld("  Tool count   : %lu\n", n_tools);
        printf_epiworld("  Neigh. count : %lu\n", neighbors.size());

    }

    return;

}

// template<typename TSeq>
// inline double & Agent<TSeq>::operator()(size_t j)
// {

//     if (model->population_data_n_features <= j)
//         throw std::logic_error("The requested feature of the agent is out of range.");

//     return *(model->population_data + j * model->size() + id);

// }

// template<typename TSeq>
// inline double & Agent<TSeq>::operator[](size_t j)
// {
//     return *(model->population_data + j * model->size() + id);
// }

template<typename TSeq>
inline Entities<TSeq> Agent<TSeq>::get_entities()
{
    return Entities<TSeq>(*this);
}

template<typename TSeq>
inline const Entities_const<TSeq> Agent<TSeq>::get_entities() const
{
    return Entities_const<TSeq>(*this);
}

#undef CHECK_COALESCE_

#endif