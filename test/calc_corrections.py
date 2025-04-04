#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep


def BGV(d):
    return -0.13 * np.exp(-3.13 * 10 ** (-2) * d) + 1.38


def BGV_inv(bgv):
    # print(bgv)
    # print((1.38 - bgv) / 0.13)
    return np.log((1.38 - bgv) / 0.13) / (-3.13 * 10 ** (-2))


if __name__ == "__main__":
    corrs_bpix_2023_v0_old_model = {1: 1.037, 2: 1.031, 3: 1.020, 4: 1.012}
    corrs_fpix_2023_v0_old_model = {1: 1.14, 2: 1.14, 3: 1.14}

    lumi_2017 = 49.97
    lumi_2018 = 65.77

    lumi = {}

    lumi["2023_v0"] = 41.97
    lumi["2023_v1"] = 62.57  # in fb^{-1}
    lumi["2023_v3"] = lumi["2023_v1"] + 10.843993752  # in fb^{-1}
    lumi["2024_v0"] = lumi["2023_v3"] + 0.000401619  # in fb^{-1}
    lumi["2024_v1"] = lumi["2024_v0"] + 29.214249009  # in fb^{-1}

    print(f"Luminosity until end 2018: {lumi_2017 + lumi_2018:>7.2f} [1/fb]")
    for tag in lumi.keys():
        print(f"Luminosity until {tag}:  {lumi_2017 + lumi_2018 + lumi[tag]:>7.2f} [1/fb]")

    # doses in Mrad
    fpix_factor_ring1 = 0.212
    fpix_factor_ring2 = 0.056

    dose = {}
    
    for tag in lumi.keys():
        dose[tag] = {}

        dose[tag]["layer1"] = 0.35 * lumi[tag]
        dose[tag]["layer2"] = 0.0717 * (lumi_2017 + lumi_2018 + lumi[tag])
        dose[tag]["layer3"] = 0.0438 * (lumi_2017 + lumi_2018 + lumi[tag])
        dose[tag]["layer4"] = 0.0236 * (lumi_2017 + lumi_2018 + lumi[tag])
        dose[tag]["ring1"]  = fpix_factor_ring1 * (lumi_2017 + lumi_2018 + lumi[tag])
        dose[tag]["ring2"]  = fpix_factor_ring2 * (lumi_2017 + lumi_2018 + lumi[tag])

    corr = {}
    avg_corr_fpix = {}
    
    for tag in lumi.keys():
        corr[tag] = {}

        corr[tag]["ring1"] = BGV(dose[tag]["ring1"]) / BGV(0)
        corr[tag]["ring2"] = BGV(dose[tag]["ring2"]) / BGV(0)
        avg_corr_fpix[tag] = round((corr[tag]["ring1"] + corr[tag]["ring2"]) / 2 * (49 / 47) , 3)


    print("\nBPix Doses:")
    for tag in lumi.keys():
        for layer in range(1,5):
            d = dose[tag][f"layer{layer}"]
            print(f"dose in Run3 until {tag} for layer {layer}: {d:>7.4f} Mrad")
        print("")


    print("\nFPix Doses:")
    for tag in lumi.keys():
        for ring in range(1,3):
            d = dose[tag][f"ring{ring}"]
            print(f"dose in Run3 until {tag} for ring {ring}: {d:>7.4f} Mrad")
        print("")

    for tag in lumi.keys():
        for ring in range(1,3):
            c = corr[tag][f"ring{ring}"]
            print(f"corr in Run3 until {tag} for ring {ring}: {c:.4f}")
        print("")

    for tag in lumi.keys():
        for ring in range(1,3):
            c = corr[tag][f"ring{ring}"]
            print(f"corr in Run3 until {tag} for ring {ring} x49/47: {c * (49 / 47):.4f}")
        print("")

    for tag in lumi.keys():
        c = avg_corr_fpix[tag]
        print(f"avg of ring1 and ring2 corr in Run3 until {tag} x49/47: {c:.4f}")
    print("")

    corrs_bpix = {}

    for tag in lumi.keys():
        corrs_bpix[tag] = {
            i: round(BGV(d) / BGV(0), 3)
            for i, d in zip(
                range(1, 5),
                [
                    dose[tag]["layer1"],
                    dose[tag]["layer2"],
                    dose[tag]["layer3"],
                    dose[tag]["layer4"]
                ]
            )
        }

    corrs_fpix = {}

    for tag in list(lumi.keys())[0:2]:
        corrs_fpix[tag] = {
            1 : avg_corr_fpix[tag],
            2 : avg_corr_fpix[tag],
            3 : avg_corr_fpix[tag],
        }

    for tag in list(lumi.keys())[2:]:
        corrs_fpix[tag] = {
            1 : round(corr[tag]["ring1"] * (49 / 47), 4),
            2 : round(corr[tag]["ring2"] * (49 / 47), 4),
        }

    print("values used for 2023_v0 (old model fpix):")
    print(f"corrs_bpix = {corrs_bpix_2023_v0_old_model}")
    print(f"corrs_fpix = {corrs_fpix_2023_v0_old_model}\n")
    for tag in lumi.keys():
        print(f"values used for {tag} new model:")
        c = corrs_bpix[tag]
        print(f"corrs_bpix = {c}")
        c = corrs_fpix[tag]
        print(f"corrs_fpix = {c}\n")

    latest_tag = list(lumi.keys())[-1]

    # plt.rcParams.update({
    #     "text.usetex": True,
    #     "font.family": "sans-serif",
    #     "font.sans-serif": "Helvetica",
    # })
    hep.style.use("CMS")
    px = 1/plt.rcParams['figure.dpi']  # pixel in inches
    fig, ax = plt.subplots(figsize=(4*400*px, 3*400*px))
    labels = [
        r'$-0.13V * e^{-0.0313Mrad^{-1} * dose} + 1.38V$',
        r'$1. + dose * 0.0045 Mrad^{-1}$',
    ]

    y_upper_lim = 1.2
    x_upper_lim = np.ceil(max(dose[latest_tag].values())/10.) * 10. + 20.
    x = np.linspace(0, x_upper_lim, 100)
    y = BGV(x) / BGV(0)
    y_old = 1 + x * 0.0045

    new_model = ax.plot(x, y, linewidth=2.0, label=labels[0], linestyle='solid')
    old_model = ax.plot(x, y_old, linewidth=2.0, label=labels[1], linestyle='dashdot')

    ax.set(xlim=(0, x_upper_lim), xticks=np.arange(0, x_upper_lim, 10),
           ylim=(1.0, y_upper_lim), yticks=np.arange(1.0, y_upper_lim, 0.05))
    ax.set_xlabel('dose /(Mrad)', loc='right')
    ax.set_ylabel('slope correction factor', loc='top')
    ax.grid(True, axis='both')

    ax.legend()

    vlines = [
        [dose[latest_tag]["layer1"], "layer 1"],
        [dose[latest_tag]["layer2"], "layer 2"],
        [dose[latest_tag]["layer3"], "layer 3"],
        [dose[latest_tag]["layer4"], "layer 4"],
        [dose[latest_tag]["ring1"], "ring 1"],
        [dose[latest_tag]["ring2"], "ring 2"],
    ]
    for vline in vlines:
        ax.axvline(x=vline[0], color='red', linestyle='--')
        ax.text(vline[0]-1, y_upper_lim + 0.005, vline[1], rotation=90, size='x-small')
    ax.text(x_upper_lim, y_upper_lim + 0.005, f"Gain Calibration {latest_tag}", horizontalalignment='right')

    plt.savefig(f"calc_corrections_{latest_tag}.png")
    plt.show()
    # input("Enter")
