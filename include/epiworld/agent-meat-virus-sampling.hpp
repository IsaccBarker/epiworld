#ifndef EPIWORLD_AGENT_MEAT_VIRUS_SAMPLING
#define EPIWORLD_AGENT_MEAT_VIRUS_SAMPLING

/**
 * @brief Functions for sampling viruses
 * 
 */
namespace sampler {

/**
 * @brief Make a function to sample from neighbors
 * 
 * This is akin to the function default_update_susceptible, with the difference
 * that it will create a function that supports excluding states from the sampling
 * frame. For example, individuals who have acquired a virus can be excluded if
 * in incubation state.
 * 
 * @tparam TSeq 
 * @param exclude unsigned vector of states that need to be excluded from the sampling
 * @return Virus<TSeq>* of the selected virus. If none selected (or none
 * available,) returns a nullptr;
 */
template<typename TSeq>
inline std::function<void(Agent<TSeq>*,Model<TSeq>*)> make_update_susceptible(
    std::vector< epiworld_fast_uint > exclude = {}
    )
{
  

    if (exclude.size() == 0u)
    {

        std::function<void(Agent<TSeq>*,Model<TSeq>*)> sampler =
            [](Agent<TSeq> * p, Model<TSeq> * m) -> void
            {

                if (p->get_n_viruses() > 0u)
                    throw std::logic_error(
                        std::string("Using the -default_update_susceptible- on agents WITH viruses makes no sense! ") +
                        std::string("Agent id ") + std::to_string(p->get_id()) +
                        std::string(" has ") + std::to_string(p->get_n_viruses()) +
                        std::string(" viruses.")
                        );

                // This computes the prob of getting any neighbor variant
                size_t nvariants_tmp = 0u;
                for (auto & neighbor: p->get_neighbors()) 
                {
                            
                    for (const VirusPtr<TSeq> & v : neighbor->get_viruses()) 
                    { 

                        #ifdef EPI_DEBUG
                        if (nvariants_tmp >= m->array_virus_tmp.size())
                            throw std::logic_error("Trying to add an extra element to a temporal array outside of the range.");
                            // printf_epiworld("N used %d\n", v.use_count());
                        #endif
                            
                        /* And it is a function of susceptibility_reduction as well */ 
                        m->array_double_tmp[nvariants_tmp] =
                            (1.0 - p->get_susceptibility_reduction(v, m)) * 
                            v->get_prob_infecting(m) * 
                            (1.0 - neighbor->get_transmission_reduction(v, m)) 
                            ; 
                    
                        m->array_virus_tmp[nvariants_tmp++] = &(*v);
                        
                    } 
                }

                // No virus to compute
                if (nvariants_tmp == 0u)
                    return;

                // Running the roulette
                int which = roulette(nvariants_tmp, m);

                if (which < 0)
                    return;

                p->add_virus(*m->array_virus_tmp[which], m);

                return; 
            };

        return sampler;

    } else {

        // Making room for the query
        std::shared_ptr<std::vector<bool>> exclude_agent_bool =
            std::make_shared<std::vector<bool>>(0);

        std::shared_ptr<std::vector<epiworld_fast_uint>> exclude_agent_bool_idx =
            std::make_shared<std::vector<epiworld_fast_uint>>(exclude);

        std::function<void(Agent<TSeq>*,Model<TSeq>*)> sampler =
            [exclude_agent_bool,exclude_agent_bool_idx](Agent<TSeq> * p, Model<TSeq> * m) -> void
            {

                // The first time we call it, we need to initialize the vector
                if (exclude_agent_bool->size() == 0u)
                {

                    exclude_agent_bool->resize(m->get_status().size(), false);
                    for (auto s : *exclude_agent_bool_idx)
                    {
                        if (s >= exclude_agent_bool->size())
                            throw std::logic_error(
                                std::string("You are trying to exclude a status that is out of range: ") +
                                std::to_string(s) + std::string(". There are only ") +
                                std::to_string(exclude_agent_bool->size()) + 
                                std::string(" statuses in the model.")
                                );

                        exclude_agent_bool->operator[](s) = true;

                    }

                }                    

                if (p->get_n_viruses() > 0u)
                    throw std::logic_error(
                        std::string("Using the -default_update_susceptible- on agents WITH viruses makes no sense! ") +
                        std::string("Agent id ") + std::to_string(p->get_id()) +
                        std::string(" has ") + std::to_string(p->get_n_viruses()) +
                        std::string(" viruses.")
                        );

                // This computes the prob of getting any neighbor variant
                size_t nvariants_tmp = 0u;
                for (auto & neighbor: p->get_neighbors()) 
                {

                    // If the status is in the list, exclude it
                    if (exclude_agent_bool->operator[](neighbor->get_status()))
                        continue;
                            
                    for (const VirusPtr<TSeq> & v : neighbor->get_viruses()) 
                    { 

                        #ifdef EPI_DEBUG
                        if (nvariants_tmp >= m->array_virus_tmp.size())
                            throw std::logic_error("Trying to add an extra element to a temporal array outside of the range.");
                            // printf_epiworld("N used %d\n", v.use_count());
                        #endif
                            
                        /* And it is a function of susceptibility_reduction as well */ 
                        m->array_double_tmp[nvariants_tmp] =
                            (1.0 - p->get_susceptibility_reduction(v, m)) * 
                            v->get_prob_infecting(m) * 
                            (1.0 - neighbor->get_transmission_reduction(v, m)) 
                            ; 
                    
                        m->array_virus_tmp[nvariants_tmp++] = &(*v);
                        
                    } 
                }

                // No virus to compute
                if (nvariants_tmp == 0u)
                    return;

                // Running the roulette
                int which = roulette(nvariants_tmp, m);

                if (which < 0)
                    return;

                p->add_virus(*m->array_virus_tmp[which], m); 

                return;

            };

        return sampler;

    }
    
}

/**
 * @brief Make a function to sample from neighbors
 * 
 * This is akin to the function default_update_susceptible, with the difference
 * that it will create a function that supports excluding states from the sampling
 * frame. For example, individuals who have acquired a virus can be excluded if
 * in incubation state.
 * 
 * @tparam TSeq 
 * @param exclude unsigned vector of states that need to be excluded from the sampling
 * @return Virus<TSeq>* of the selected virus. If none selected (or none
 * available,) returns a nullptr;
 */
template<typename TSeq = int>
inline std::function<Virus<TSeq>*(Agent<TSeq>*,Model<TSeq>*)> make_sample_virus_neighbors(
    std::vector< epiworld_fast_uint > exclude = {}
)
{
    if (exclude.size() == 0u)
    {

        std::function<Virus<TSeq>*(Agent<TSeq>*,Model<TSeq>*)> res = 
            [](Agent<TSeq> * p, Model<TSeq> * m) -> Virus<TSeq>* {

                if (p->get_n_viruses() > 0u)
                    throw std::logic_error(
                        std::string("Using the -default_update_susceptible- on agents WITH viruses makes no sense! ") +
                        std::string("Agent id ") + std::to_string(p->get_id()) +
                        std::string(" has ") + std::to_string(p->get_n_viruses()) +
                        std::string(" viruses.")
                        );

                // This computes the prob of getting any neighbor variant
                size_t nvariants_tmp = 0u;
                for (auto & neighbor: p->get_neighbors()) 
                {
                            
                    for (const VirusPtr<TSeq> & v : neighbor->get_viruses()) 
                    { 

                        #ifdef EPI_DEBUG
                        if (nvariants_tmp >= m->array_virus_tmp.size())
                            throw std::logic_error("Trying to add an extra element to a temporal array outside of the range.");
                            // printf_epiworld("N used %d\n", v.use_count());
                        #endif
                            
                        /* And it is a function of susceptibility_reduction as well */ 
                        m->array_double_tmp[nvariants_tmp] =
                            (1.0 - p->get_susceptibility_reduction(v, m)) * 
                            v->get_prob_infecting(m) * 
                            (1.0 - neighbor->get_transmission_reduction(v, m)) 
                            ; 
                    
                        m->array_virus_tmp[nvariants_tmp++] = &(*v);
                        
                    } 
                }

                // No virus to compute
                if (nvariants_tmp == 0u)
                    return nullptr;

                // Running the roulette
                int which = roulette(nvariants_tmp, m);

                if (which < 0)
                    return nullptr;

                return m->array_virus_tmp[which]; 

            };

        return res;


    } else {

        // Making room for the query
        std::shared_ptr<std::vector<bool>> exclude_agent_bool =
            std::make_shared<std::vector<bool>>(0);

        std::shared_ptr<std::vector<epiworld_fast_uint>> exclude_agent_bool_idx =
            std::make_shared<std::vector<epiworld_fast_uint>>(exclude);


        std::function<Virus<TSeq>*(Agent<TSeq>*,Model<TSeq>*)> res = 
            [exclude_agent_bool,exclude_agent_bool_idx](Agent<TSeq> * p, Model<TSeq> * m) -> Virus<TSeq>* {

                // The first time we call it, we need to initialize the vector
                if (exclude_agent_bool->size() == 0u)
                {

                    exclude_agent_bool->resize(m->get_status().size(), false);
                    for (auto s : *exclude_agent_bool_idx)
                    {
                        if (s >= exclude_agent_bool->size())
                            throw std::logic_error(
                                std::string("You are trying to exclude a status that is out of range: ") +
                                std::to_string(s) + std::string(". There are only ") +
                                std::to_string(exclude_agent_bool->size()) + 
                                std::string(" statuses in the model.")
                                );

                        exclude_agent_bool->operator[](s) = true;

                    }

                }    
                
                if (p->get_n_viruses() > 0u)
                    throw std::logic_error(
                        std::string("Using the -default_update_susceptible- on agents WITH viruses makes no sense! ") +
                        std::string("Agent id ") + std::to_string(p->get_id()) +
                        std::string(" has ") + std::to_string(p->get_n_viruses()) +
                        std::string(" viruses.")
                        );

                // This computes the prob of getting any neighbor variant
                size_t nvariants_tmp = 0u;
                for (auto & neighbor: p->get_neighbors()) 
                {

                    // If the status is in the list, exclude it
                    if (exclude_agent_bool->operator[](neighbor->get_status()))
                        continue;
                            
                    for (const VirusPtr<TSeq> & v : neighbor->get_viruses()) 
                    { 

                        #ifdef EPI_DEBUG
                        if (nvariants_tmp >= m->array_virus_tmp.size())
                            throw std::logic_error("Trying to add an extra element to a temporal array outside of the range.");
                            // printf_epiworld("N used %d\n", v.use_count());
                        #endif
                            
                        /* And it is a function of susceptibility_reduction as well */ 
                        m->array_double_tmp[nvariants_tmp] =
                            (1.0 - p->get_susceptibility_reduction(v, m)) * 
                            v->get_prob_infecting(m) * 
                            (1.0 - neighbor->get_transmission_reduction(v, m)) 
                            ; 
                    
                        m->array_virus_tmp[nvariants_tmp++] = &(*v);
                        
                    } 
                }

                // No virus to compute
                if (nvariants_tmp == 0u)
                    return nullptr;

                // Running the roulette
                int which = roulette(nvariants_tmp, m);

                if (which < 0)
                    return nullptr;

                return m->array_virus_tmp[which]; 

            };

        return res;

    }

}

/**
 * @brief Sample from neighbors pool of viruses (at most one)
 * 
 * This function samples at most one virus from the pool of
 * viruses from its neighbors. If no virus is selected, the function
 * returns a `nullptr`, otherwise it returns a pointer to the
 * selected virus.
 * 
 * This can be used to build a new update function (EPI_NEW_UPDATEFUN.)
 * 
 * @tparam TSeq 
 * @param p Pointer to person 
 * @param m Pointer to the model
 * @return Virus<TSeq>* of the selected virus. If none selected (or none
 * available,) returns a nullptr;
 */
template<typename TSeq = int>
inline Virus<TSeq> * sample_virus_single(Agent<TSeq> * p, Model<TSeq> * m)
{

    if (p->get_n_viruses() > 0u)
        throw std::logic_error(
            std::string("Using the -default_update_susceptible- on agents WITH viruses makes no sense! ") +
            std::string("Agent id ") + std::to_string(p->get_id()) +
            std::string(" has ") + std::to_string(p->get_n_viruses()) +
            std::string(" viruses.")
            );

    // This computes the prob of getting any neighbor variant
    size_t nvariants_tmp = 0u;
    for (auto & neighbor: p->get_neighbors()) 
    {   
        #ifdef EPI_DEBUG
        int _vcount_neigh = 0;
        #endif                 
        for (const VirusPtr<TSeq> & v : neighbor->get_viruses()) 
        { 

            #ifdef EPI_DEBUG
            if (nvariants_tmp >= m->array_virus_tmp.size())
                throw std::logic_error("Trying to add an extra element to a temporal array outside of the range.");
                // printf_epiworld("N used %d\n", v.use_count());
            #endif
                
            /* And it is a function of susceptibility_reduction as well */ 
            m->array_double_tmp[nvariants_tmp] =
                (1.0 - p->get_susceptibility_reduction(v, m)) * 
                v->get_prob_infecting(m) * 
                (1.0 - neighbor->get_transmission_reduction(v, m)) 
                ; 
        
            m->array_virus_tmp[nvariants_tmp++] = &(*v);

            #ifdef EPI_DEBUG
            if (
                (m->array_double_tmp[nvariants_tmp - 1] < 0.0) |
                (m->array_double_tmp[nvariants_tmp - 1] > 1.0)
                )
            {
                printf_epiworld(
                    "[epi-debug] Agent %i's virus %i has transmission prob outside of [0, 1]: %.4f!\n",
                    static_cast<int>(neighbor->get_id()),
                    static_cast<int>(_vcount_neigh++),
                    m->array_double_tmp[nvariants_tmp - 1]
                    );
            }
            #endif
            
        } 
    }


    // No virus to compute
    if (nvariants_tmp == 0u)
        return nullptr;

    #ifdef EPI_DEBUG
    m->get_db().n_transmissions_potential++;
    #endif

    // Running the roulette
    int which = roulette(nvariants_tmp, m);

    if (which < 0)
        return nullptr;

    #ifdef EPI_DEBUG
    m->get_db().n_transmissions_today++;
    #endif

    return m->array_virus_tmp[which]; 
    
}

}

#endif