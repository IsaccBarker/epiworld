#include "../../epiworld.hpp"

////////////////////////////////////////////////////////////////////////////////
/**
 * DETAILS OF THE MODEL
 * 
 * # ROWS
 * ====================
 * 
 * ## Optimistic waning of protection against infection
 * 
 * - Slow immune waning, median transition time to partially immune state =
 *  10 months
 * 
 * - In the partially immune state, there is a 40% reduciton in protection from
 *  baseline levels reported immediately after exposure (vaccination or
 *  infection.)
 * 
 * ## Pessimistic waning of protection against infection
 * 
 * - Fast immune waning, median transition time to partially immune state =
 *  4 months.
 * 
 * - In the partially immune state, there is a 60% reduction in protection from
 *  baseline levels reported immediately after exposure (vaccination or
 *  infection.)
 * 
 * # COLUMNS
 * ====================
 * 
 * ## No new variant
 * 
 * - Projections are initialized with the mix of strains circulating at the
 *  start of the projection period.
 * 
 * - New variant X emerges on May 1st, 2022. There is a continuous influx of 50
 *  weekly infections of variant X for the following 16 wks. Variant X has
 *  30% immune escape, and the same intrinsic transmissibility and severity as
 *  Omicron.
 */

// Designing variants ------------------------------------------------------

int main()
{

    // Delta
    epiworld::Virus variant_delta("Delta");
    variant_delta.set_post_immunity(.4);
    variant_delta.set_prob_infecting(.9);
    variant_delta.set_prob_recovery(.3);

    // Omicron
    epiworld::Virus variant_omicron("Omicron");
    variant_omicron.set_post_immunity(.4);
    variant_omicron.set_prob_infecting(.9);
    variant_omicron.set_prob_recovery(.3);

    // Variant X
    epiworld::Virus variant_x("X");
    variant_x.set_post_immunity(.4);
    variant_x.set_prob_infecting(.9);
    variant_x.set_prob_recovery(.3);

    EPI_NEW_GLOBALFUN_LAMBDA(global_variant_x,bool)
    {
        // Identify an individual that will acquire the new variant
        int total_infected = m->get_db().get_today_total("infected");
        int who = static_cast<int>( total_infected * m->runif() );

        const auto & queue = m->get_queue();

        int cum_counts = 0;
        auto population = (*m->get_population());
        for (size_t i = 0u; i < m->size(); ++i)
            if (queue[i] > 0u)
            {
                // Is this person infected
                auto & person = population[i];
                
                if (person.has_virus("Omicron"))
                {
                    if (++cum_counts == who)
                    {
                        // Replacing the virus
                        person.get_virus(0) = variant_x;
                        break;
                    }
                }
                
            }

    };

    // Setup
    epiworld::Model<> model;
    model.population_smallworld();
    model.add_virus(epiworld::Virus<>(), 0.1);
    model.add_tool(epiworld::Tool<>(), 1.0);

    // Initializing and running the model
    model.init(100, 226);
    model.run();

    // Printing the output
    model.print();

    return 0;

}