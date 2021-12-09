#ifndef EPIWORLD_PERSON_HPP
#define EPIWORLD_PERSON_HPP

template<typename TSeq>
class Virus;

template<typename TSeq>
class Tool;

template<typename TSeq>
class PersonViruses;

template<typename TSeq>
class PersonTools;

template<typename TSeq>
class Person {
    friend class Model<TSeq>;
    friend class Tool<TSeq>;

private:
    Model<TSeq> * model;
    PersonViruses<TSeq> viruses;
    PersonTools<TSeq> tools;
    std::vector< Person<TSeq> * > neighbors;

public:

    Person();

    void add_tool(int d, Tool<TSeq> tool);
    void add_virus(int d, Virus<TSeq> virus);

    double get_efficacy(Virus<TSeq> * v);
    double get_recovery(Virus<TSeq> * v);
    double get_death(Virus<TSeq> * v);
    
    std::mt19937 * get_rand_endgine();
    Model<TSeq> * get_model(); 

    Virus<TSeq> & get_virus(int i);
    PersonViruses<TSeq> & get_viruses();

    void mutate_virus();
    void add_neighbor(Person<TSeq> * p);

    void update_status();

};

template<typename TSeq>
inline Person<TSeq>::Person() {
    tools.person = this;
    viruses.person = this;
}

template<typename TSeq>
inline void Person<TSeq>::add_tool(
    int d,
    Tool<TSeq> tool
) {
    tools.add_tool(d, tool);
}

template<typename TSeq>
inline void Person<TSeq>::add_virus(
    int d,
    Virus<TSeq> virus
) {
    viruses.add_virus(d, virus);
}

template<typename TSeq>
inline double Person<TSeq>::get_efficacy(
    Virus<TSeq> * v
) {
    return tools.get_efficacy(v);
}

template<typename TSeq>
inline double Person<TSeq>::get_recovery(
    Virus<TSeq> * v
) {
    return tools.get_recovery(v);
}

template<typename TSeq>
inline double Person<TSeq>::get_death(
    Virus<TSeq> * v
) {
    return tools.get_death(v);
}

template<typename TSeq>
inline std::mt19937 * Person<TSeq>::get_rand_endgine() {
    return model->get_rand_endgine();
}

template<typename TSeq>
inline Model<TSeq> * Person<TSeq>::get_model() {
    return model;
}

template<typename TSeq>
inline PersonViruses<TSeq> & Person<TSeq>::get_viruses() {
    return viruses;
}

template<typename TSeq>
inline Virus<TSeq> & Person<TSeq>::get_virus(int i) {
    return viruses(i);
}

template<typename TSeq>
inline void Person<TSeq>::mutate_virus() {
    viruses.mutate();
}

template<typename TSeq>
inline void Person<TSeq>::add_neighbor(
    Person<TSeq> * p
) {
    neighbors.push_back(p);
    p->neighbors.push_back(this);
}


template<typename TSeq>
inline void Person<TSeq>::update_status() {

    // Step 1: Compute the individual efficacies
    std::vector<double> probs;
    std::vector< Virus<TSeq>* > variants;
    std::vector< int > certain_infection; ///< Viruses for which there's no chance of escape

    // Computing the efficacy
    double p_none = 1.0; ///< Product of all (1 - efficacy)
    double tmp_efficacy;
    for (int n = 0; n < neighbors.size(); ++n)
    {

        Person<TSeq> * neighbor = neighbors[n];
        PersonViruses<TSeq> nviruses = neighbor->get_viruses();
        
        // Now over the neighbor's viruses
        for (int v = 0; v < nviruses.size(); ++v)
        {

            // Computing the corresponding efficacy
            Virus<TSeq> * tmp_v = &(neighbor->get_virus(v));
            tmp_efficacy = get_efficacy(tmp_v);
            probs.push_back(tmp_efficacy);
            variants.push_back(tmp_v);

            // Adding to the product
            p_none *= tmp_efficacy;

            // Is it impossible to escape?
            if (tmp_efficacy < 1e-100)
                certain_infection.push_back(probs.size() - 1);


        }
    }

    // Case in which infection is certain. All certain infections have
    // equal chance of taking the individual
    double r = model->runif();
    if (certain_infection.size() > 0)
    {
        add_virus(model->today(), *variants[
            certain_infection[std::floor(r * certain_infection.size())]
        ]);

        // And we go back
        return;
    }

    // Step 2: Calculating the prob of none or single
    double p_none_or_single = p_none;
    for (int v = 0; v < probs.size(); ++v)
        if (probs[v] > 1e-15)
            p_none_or_single += p_none / probs[v] * (1 - probs[v]);

    
    // Step 3: Roulette
    double cumsum = p_none / p_none_or_single;
    

    // If this is the case, then nothing happens
    if (r < cumsum)
        return;

    for (int v = 0; v < probs.size(); ++v)
    {
        // If it yield here, then bingo, the individual will acquire the disease
        cumsum += (1 - probs[v])/p_none_or_single;
        if (r < cumsum)
        {
            add_virus(model->today(), *variants[v]);
            return;
        }
        
    }

    // We shouldn't have reached this place
    throw std::logic_error(
        "Your calculations are wrong. This should't happen:\ncumsum : " +
        std::to_string(cumsum) + "\nr      : " + std::to_string(r) + "\n"
        );



}

#endif